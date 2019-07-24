/*
Copyright © 2019 InAccel

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <rabit/rabit.h>
#include <xgboost/tree_updater.h>
#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>

#include "../common/random.h"
#include "../common/bitmap.h"
#include "../common/timer.h"
#include "../tree/split_evaluator.h"
#include "../tree/param.h"
#include <coral-api/coral.h>
#include <coral-api/request.h>
#include <coral-api/allocator.h>

namespace xgboost {
namespace inaccel {

using xgboost::tree::GradStats;
using xgboost::tree::SplitEvaluator;
using xgboost::tree::TrainParam;

DMLC_REGISTRY_FILE_TAG(updater_fpga_coral);

// actual builder that runs the algorithm
// distributed column maker
class DistFpgaMaker : public TreeUpdater {
 public:
	void Configure(const Args& args) override {
		param_.InitAllowUnknown(args);
		pruner_.reset(TreeUpdater::Create("prune", tparam_));
		pruner_->Configure(args);
		spliteval_.reset(SplitEvaluator::Create(param_.split_evaluator));
		spliteval_->Init(args);
		is_dmat_fpga_initialized_ = false;
		nRequests_ = 2;
		for(auto p : args)
			if(p.first == "nRequests") nRequests_ = std::stoi(p.second);
	}
	char const* Name() const override {
		return "grow_fpga";
	}
	void Update(HostDeviceVector<GradientPair> *gpair, DMatrix* dmat,
				const std::vector<RegTree*> &trees) override {
		monitor_.Init("Update");
		CHECK_EQ(trees.size(), 1U) << "DistFpgaMaker: only support one tree at a time";
		const auto nrow = static_cast<uint32_t>(dmat->Info().num_row_);
		const auto ncol = static_cast<uint32_t>(dmat->Info().num_col_);
		if (is_dmat_fpga_initialized_ == false) {
			monitor_.Start("Init dmat_fpga");
			max_rows_ = 0;
			for (const auto &batch : dmat->GetSortedColumnBatches()) {
				for (uint32_t cidx = 0; cidx < ncol; cidx++) {
					auto col = batch[cidx];
					auto rows = static_cast<uint32_t>(col.size());
					if(rows > max_rows_) max_rows_ = rows;
				}
			}
			dmat_fpga_.resize(nRequests_);
			req_cols_.resize(nRequests_+1);
			req_cols_[0] = 0;
			uint32_t ncol_div = ncol/nRequests_;
			uint32_t ncol_mod = ncol%nRequests_;
			auto nrow_mlt = max_rows_*8;
			Entry invalid;
			invalid.fvalue = 0;
			invalid.index = -1;
			for(uint32_t req = 0; req<nRequests_; req++) {
				req_cols_[req+1] = req_cols_[req] + ncol_div + ((ncol_mod>0)?1:0);
				ncol_mod-=((ncol_mod>0)?1:0);
				auto ncol_req = req_cols_[req+1] - req_cols_[req];
				auto ncol_mlt = ncol_req/8 + ((ncol_req%8)>0?1:0);
				dmat_fpga_[req].resize(ncol_mlt*nrow_mlt);
				std::fill(dmat_fpga_[req].begin(),dmat_fpga_[req].end(),invalid);
				for (const auto &batch : dmat->GetSortedColumnBatches()) {
					#pragma omp parallel for schedule(static)
					for (uint32_t cidx = req_cols_[req]; cidx < req_cols_[req+1]; cidx++) {
						auto col = batch[cidx];
						auto rblock_idx = (cidx-req_cols_[req])%8;
						auto ncidx = (cidx-req_cols_[req])/8;
						const auto ndata = static_cast<uint32_t>(col.size());
						for (uint32_t ridx = 0; ridx < ndata; ridx++) {
							const Entry e = col[ridx];
							auto rblock = ridx*8;
							dmat_fpga_[req][ncidx*nrow_mlt + rblock + rblock_idx] = e;
						}
					}
				}
			}
			is_dmat_fpga_initialized_ = true;
			monitor_.Stop("Init dmat_fpga");
		}
		Builder builder( nrow, ncol, max_rows_, nRequests_, param_, monitor_,
						 std::unique_ptr<SplitEvaluator>(spliteval_->GetHostClone()));
		monitor_.Start("Init gpair_fpga");
		const std::vector<GradientPair>& gpair_h = gpair->ConstHostVector();
		size_t gpair_fpga_size = gpair_h.size() + (((gpair_h.size()%8)>0)?(8 - (gpair_h.size()%8)):0);
		gpair_fpga_.resize(0);
		gpair_fpga_.shrink_to_fit();
		gpair_fpga_.resize(gpair_fpga_size);
		gpair_fpga_.assign(gpair_h.begin(),gpair_h.end());
		monitor_.Stop("Init gpair_fpga");
		monitor_.Start("builder Update");
		builder.Update(gpair->ConstHostVector(), gpair_fpga_, dmat, dmat_fpga_, req_cols_, trees[0]);
		monitor_.Stop("builder Update");
		monitor_.Start("pruner Update");
		pruner_->Update(gpair, dmat, trees);
		monitor_.Stop("pruner Update");
		builder.UpdatePosition(dmat, *trees[0]);
	}
 protected:
	common::Monitor monitor_;
	unsigned nRequests_;
	uint32_t max_rows_;
	TrainParam param_;
	std::unique_ptr<SplitEvaluator> spliteval_;
	std::unique_ptr<TreeUpdater> pruner_;
	bool is_dmat_fpga_initialized_;
	//cubes
	std::vector<::inaccel::vector<Entry>> dmat_fpga_;
	std::vector<uint32_t> req_cols_;
	::inaccel::vector<GradientPair> gpair_fpga_;
	// data structure
	struct XGBOOST_ALIGNAS(8) GradStatsInAccel {
	  float sum_grad;
	  float sum_hess;
	  float GetGrad() const { return sum_grad; }
	  float GetHess() const { return sum_hess; }

	  GradStatsInAccel() : sum_grad{0}, sum_hess{0} {
	    static_assert(sizeof(GradStatsInAccel) == 8,
	                  "Size of GradStatsInAccel is not 8 bytes.");
	  }

	  template <typename GpairT>
	  explicit GradStatsInAccel(const GpairT &sum)
	      : sum_grad(sum.GetGrad()), sum_hess(sum.GetHess()) {}
	  explicit GradStatsInAccel(const float grad, const float hess)
	      : sum_grad(grad), sum_hess(hess) {}
	  inline void Add(float grad, float hess) {
	    sum_grad += grad;
	    sum_hess += hess;
	  }
	  template <typename GpairT>
	  inline void Add(const GpairT &p) { this->Add(p.GetGrad(), p.GetHess()); }
	  inline static void Reduce(GradStatsInAccel& a, const GradStatsInAccel& b) { // NOLINT(*)
	    a.Add(b);
	  }
	  inline void SetSubstract(const GradStatsInAccel& a, const GradStatsInAccel& b) {
	    sum_grad = a.sum_grad - b.sum_grad;
	    sum_hess = a.sum_hess - b.sum_hess;
	  }
	  inline bool Empty() const { return sum_hess == 0.0; }
	};
	struct XGBOOST_ALIGNAS(32) SplitEntryInAccelRet {
	  float loss_chg;
	  unsigned sindex;
	  float split_value;
	  float left_sum_grad;
	  float left_sum_hess;
	  unsigned nu1;
	  unsigned nu2;
	  unsigned nu3;
	};
	struct SplitEntryInAccel {
	  float loss_chg{0.0f};
	  unsigned sindex{0};
	  float split_value{0.0f};
	  GradStatsInAccel left_sum;
	  GradStatsInAccel right_sum;
	  SplitEntryInAccel()  = default;
	  SplitEntryInAccel(const GradStatsInAccel& parent,
	  					const SplitEntryInAccelRet& new_split, const uint32_t& offset)
	  {
		  this->loss_chg = new_split.loss_chg;
		  this->sindex = new_split.sindex + offset;
		  this->split_value = new_split.split_value;
		  this->left_sum.sum_grad = new_split.left_sum_grad;
		  this->left_sum.sum_hess = new_split.left_sum_hess;
		  this->right_sum.sum_grad = parent.sum_grad - new_split.left_sum_grad;
		  this->right_sum.sum_hess = parent.sum_hess - new_split.left_sum_hess;
	  }
	  inline bool NeedReplace(float new_loss_chg, unsigned split_index) const {
	    if (this->SplitIndex() <= split_index) {
	      return new_loss_chg > this->loss_chg;
	    } else {
	      return !(this->loss_chg > new_loss_chg);
	    }
	  }
	  inline bool Update(const SplitEntryInAccel &e) {
	    if (this->NeedReplace(e.loss_chg, e.SplitIndex())) {
	      this->loss_chg = e.loss_chg;
	      this->sindex = e.sindex;
	      this->split_value = e.split_value;
	      this->left_sum = e.left_sum;
	      this->right_sum = e.right_sum;
	      return true;
	    } else {
	      return false;
	    }
	  }
	  inline bool Update(float new_loss_chg, unsigned split_index,
	                     float new_split_value, bool default_left,
	                     const GradStatsInAccel &left_sum, const GradStatsInAccel &right_sum) {
	    if (this->NeedReplace(new_loss_chg, split_index)) {
	      this->loss_chg = new_loss_chg;
	      if (default_left) {
	        split_index |= (1U << 31);
	      }
	      this->sindex = split_index;
	      this->split_value = new_split_value;
	      this->left_sum = left_sum;
	      this->right_sum = right_sum;
	      return true;
	    } else {
	      return false;
	    }
	  }
	  inline static void Reduce(SplitEntryInAccel &dst, // NOLINT(*)
	                            const SplitEntryInAccel &src) { // NOLINT(*)
	    dst.Update(src);
	  }
	  inline unsigned SplitIndex() const { return sindex & ((1U << 31) - 1U); }
	  inline bool DefaultLeft() const { return (sindex >> 31) != 0; }
	};
	struct ThreadEntryInAccel {
		GradStatsInAccel stats;
		GradStatsInAccel stats_extra;
		float last_fvalue;
		float first_fvalue;
		SplitEntryInAccel best;
		ThreadEntryInAccel() : last_fvalue{0}, first_fvalue{0} {}
	};
	struct NodeEntryInAccel {
		GradStatsInAccel stats;
		float root_gain;
		float weight;
		SplitEntryInAccel best;
		NodeEntryInAccel() : root_gain{0.0f}, weight{0.0f} {}
	};
 private:
	class Builder {
	 protected:
	 	unsigned nrows_;
	 	unsigned ncols_;
	 	unsigned max_rows_;
	 	unsigned nRequests_;

		const TrainParam& param_;
		common::Monitor& monitor_;
		const int nthread_;
		common::ColumnSampler column_sampler_;
		std::vector<int> position_;
		::inaccel::vector<short int> position_fpga_;
		std::vector< std::vector<ThreadEntryInAccel> > stemp_;
		std::vector<NodeEntryInAccel> snode_;
		::inaccel::vector<GradStatsInAccel> snode_stats_;
		::inaccel::vector<float> snode_rg_;
		std::vector<::inaccel::vector<char>> feat_valid_fpga_;
		std::vector<int> qexpand_;
		std::vector<int> node2workindex_;
		std::unique_ptr<SplitEvaluator> spliteval_;
	 private:
		common::BitMap bitmap_;
		std::vector<int> boolmap_;
		rabit::Reducer<SplitEntryInAccel, SplitEntryInAccel::Reduce> reducer_;
	 public:
		// constructor
		explicit Builder( unsigned nrow, unsigned ncol, unsigned max_rows, unsigned nRequests,
						  const TrainParam& param, common::Monitor& monitor,
						  std::unique_ptr<SplitEvaluator> spliteval)
				: nrows_(nrow), ncols_(ncol), max_rows_(max_rows), nRequests_(nRequests), param_(param),
				  monitor_(monitor), nthread_(omp_get_max_threads()), spliteval_(std::move(spliteval)) {}
		// update one tree, growing
		virtual void Update(const std::vector<GradientPair>& gpair,
							const ::inaccel::vector<GradientPair>& gpair_fpga,
							DMatrix* p_fmat,
							const std::vector<::inaccel::vector<Entry>>& dmat_fpga,
							const std::vector<uint32_t> &req_cols,
							RegTree* p_tree) {
			monitor_.Init("Builder");
			monitor_.Start("Builder Init");
			std::vector<int> newnodes;
			this->InitData(gpair, *p_fmat, *p_tree);
			this->InitNewNode(qexpand_, gpair, *p_fmat, *p_tree);
			monitor_.Stop("Builder Init");
			for (int depth = 0; depth < param_.max_depth; ++depth) {
				monitor_.Start("Builder Create Cubes");
				this->CreateCubes( depth, *p_tree, req_cols);
				monitor_.Stop("Builder Create Cubes");
				monitor_.Start("Builder Find Splits");
				this->FindSplit( qexpand_, gpair_fpga, dmat_fpga, req_cols, p_tree);
				monitor_.Stop("Builder Find Splits");
				monitor_.Start("Builder Update Tree");
				this->ResetPosition(qexpand_, p_fmat, *p_tree);
				this->UpdateQueueExpand(*p_tree, qexpand_, &newnodes);
				this->InitNewNode(newnodes, gpair, *p_fmat, *p_tree);
				for (auto nid : qexpand_) {
					if ((*p_tree)[nid].IsLeaf()) {
						continue;
					}
					int cleft = (*p_tree)[nid].LeftChild();
					int cright = (*p_tree)[nid].RightChild();
					spliteval_->AddSplit(nid, cleft, cright, snode_[nid].best.SplitIndex(),
										 snode_[cleft].weight, snode_[cright].weight);
				}
				qexpand_ = newnodes;
				monitor_.Stop("Builder Update Tree");
				// if nothing left to be expand, break
				if (qexpand_.size() == 0) break;
			}
			// set all the rest expanding nodes to leaf
			for (const int nid : qexpand_) {
				(*p_tree)[nid].SetLeaf(snode_[nid].weight * param_.learning_rate);
			}
			// remember auxiliary statistics in the tree node
			for (int nid = 0; nid < p_tree->param.num_nodes; ++nid) {
				p_tree->Stat(nid).loss_chg = snode_[nid].best.loss_chg;
				p_tree->Stat(nid).base_weight = snode_[nid].weight;
				p_tree->Stat(nid).sum_hess = static_cast<float>(snode_[nid].stats.sum_hess);
			}
		}
		inline void InitData(const std::vector<GradientPair>& gpair, const DMatrix& fmat,
							 const RegTree& tree) {
			CHECK_EQ(tree.param.num_nodes, tree.param.num_roots) << "FpgaMaker: can only grow new tree";
			const std::vector<unsigned>& root_index = fmat.Info().root_index_;
			// setup position
			position_.resize(gpair.size());
			CHECK_EQ(nrows_, position_.size());
			if (root_index.size() == 0) {
				std::fill(position_.begin(), position_.end(), 0);
			} else {
				for (size_t ridx = 0; ridx <	position_.size(); ++ridx) {
					position_[ridx] = root_index[ridx];
					CHECK_LT(root_index[ridx], (unsigned)tree.param.num_roots);
				}
			}
			// mark delete for the deleted datas
			for (size_t ridx = 0; ridx < position_.size(); ++ridx) {
				if (gpair[ridx].GetHess() < 0.0f) position_[ridx] = ~position_[ridx];
			}
			// mark subsample
			if (param_.subsample < 1.0f) {
				std::bernoulli_distribution coin_flip(param_.subsample);
				auto& rnd = common::GlobalRandom();
				for (size_t ridx = 0; ridx < position_.size(); ++ridx) {
					if (gpair[ridx].GetHess() < 0.0f) continue;
					if (!coin_flip(rnd)) position_[ridx] = ~position_[ridx];
				}
			}
			column_sampler_.Init(ncols_, param_.colsample_bynode,
								 param_.colsample_bylevel, param_.colsample_bytree);
			// setup temp space for each thread
			// reserve a small space
			stemp_.clear();
			stemp_.resize(this->nthread_, std::vector<ThreadEntryInAccel>());
			for (auto& i : stemp_) {
				i.clear(); i.reserve(256);
			}
			snode_.reserve(256);
			// expand query
			qexpand_.reserve(256); qexpand_.clear();
			for (int i = 0; i < tree.param.num_roots; ++i) {
				qexpand_.push_back(i);
			}
			feat_valid_fpga_.resize(nRequests_);
		}
		inline void InitNewNode(const std::vector<int>& qexpand,
								const std::vector<GradientPair>& gpair,
								const DMatrix& fmat,
								const RegTree& tree) {
			{
				// setup statistics space for each tree node
				for (auto& i : stemp_) {
					i.resize(tree.param.num_nodes, ThreadEntryInAccel());
				}
				snode_.resize(tree.param.num_nodes, NodeEntryInAccel());
			}
			// setup position
			#pragma omp parallel for schedule(static)
			for (uint32_t ridx = 0; ridx < nrows_; ++ridx) {
				const int tid = omp_get_thread_num();
				if (position_[ridx] < 0) continue;
				stemp_[tid][position_[ridx]].stats.Add(gpair[ridx]);
			}
			// sum the per thread statistics together
			for (int nid : qexpand) {
				GradStatsInAccel stats;
				for (auto& s : stemp_) {
					stats.Add(s[nid].stats);
				}
				// update node statistics
				snode_[nid].stats = stats;
			}
			// calculating the weights
			for (int nid : qexpand) {
				uint32_t parentid = tree[nid].Parent();
				GradStats nstats(snode_[nid].stats);
				snode_[nid].weight = static_cast<float>(
						spliteval_->ComputeWeight(parentid, nstats));
				snode_[nid].root_gain = static_cast<float>(
						spliteval_->ComputeScore(parentid, nstats, snode_[nid].weight));
			}
		}
		inline void CreateCubes( int depth, const RegTree& tree, const std::vector<uint32_t> &req_cols)
		{
			//node cube creation
			//create node2workindex vector, which maps new nodes to positions [0,new_nodes_num)
			node2workindex_.resize(tree.param.num_nodes);
			std::fill(node2workindex_.begin(), node2workindex_.end(), -1);
			for (size_t i = 0; i < qexpand_.size(); ++i)
				node2workindex_[qexpand_[i]] = static_cast<int>(i);
			//create position_fpga_ cube with nrow size, that contains the work index of each entry
			//allign to 32 int16_t (32*2B = 64B)
			size_t position_fpga_size = position_.size() + (((position_.size()%32)>0)?(32 - (position_.size()%32)):0);
			position_fpga_.resize(0); 		//size = 0
			position_fpga_.shrink_to_fit();	//deallocate memory, to delete cube
			position_fpga_.resize(position_fpga_size); //allocate memory to create cube
			std::fill(position_fpga_.begin(),position_fpga_.end(),-1);
			for (size_t i =0; i<position_.size(); i++)
				if(position_[i] >= 0) //if position is active get work idx
					position_fpga_[i] = node2workindex_[position_[i]];
			//allign to 8 GradStats (8*8B = 64B)
			size_t snode_stats_size = qexpand_.size() + (((qexpand_.size()%8)>0)?(8 - (qexpand_.size()%8)):0);
			snode_stats_.resize(0); 		//size = 0
			snode_stats_.shrink_to_fit();	//deallocate memory, to delete cube
			snode_stats_.resize(snode_stats_size); //allocate memory to create cube
			//allign to 16 floats (16*4B = 64B)
			size_t snode_rg_size = qexpand_.size() + (((qexpand_.size()%16)>0)?(16 - (qexpand_.size()%16)):0);
			snode_rg_.resize(0); 		//size = 0
			snode_rg_.shrink_to_fit();	//deallocate memory, to delete cube
			snode_rg_.resize(snode_rg_size); //allocate memory to create cube
			for (size_t i = 0; i < qexpand_.size(); ++i)
			{
				snode_stats_[i] = snode_[qexpand_[i]].stats;
				snode_rg_[i] = snode_[qexpand_[i]].root_gain;
			}
			//feature cube creation
			//get valid features
			auto feat_set = column_sampler_.GetFeatureSet(depth);
			//create vectors with 1 in each valid feature pos and 0 in each invalid feature pos
			for(uint32_t req = 0; req<nRequests_; req++)
			{
				uint32_t nfeatures_req = req_cols[req+1] - req_cols[req];
				uint32_t fsize = nfeatures_req/8 + ((nfeatures_req%8>0)?1:0);
				feat_valid_fpga_[req].resize(0); 		//size = 0
				feat_valid_fpga_[req].shrink_to_fit();	//deallocate memory, to delete cube
				feat_valid_fpga_[req].resize(fsize); //allocate memory to create cube
				std::fill(feat_valid_fpga_[req].begin(),feat_valid_fpga_[req].end(),0);
			}
			for(uint32_t fid : feat_set->HostVector())
			{
				//calculate which req this fid belongs to
				uint32_t req = 0;
				while (fid >= req_cols[req+1]) req++;
				//shift the fid to this req's range
				uint32_t fid_shifted = fid - req_cols[req];
				//calculate which block to access
				uint32_t block = fid_shifted/8;
				//calculate the position inside the block
				uint32_t block_offset = fid_shifted%8;
				feat_valid_fpga_[req][block] |= (1<<block_offset);
			}
		}
		inline void FindSplit(  const std::vector<int> &qexpand,
								const ::inaccel::vector<GradientPair>& gpair_fpga,
								const std::vector<::inaccel::vector<Entry>>& dmat_fpga,
								const std::vector<uint32_t> &req_cols,
								RegTree *p_tree) {
			size_t qexpand_size_alligned = qexpand.size() + (qexpand.size()%2);
			CHECK_LE(qexpand.size(),2048) << 
				"More than 2048 new nodes were requested. Please reduce max depth";
			std::vector<::inaccel::vector<SplitEntryInAccelRet>> best_split;
			std::vector<::inaccel::Request> requests;
			best_split.resize(nRequests_);
			for(uint32_t req = 0; req<nRequests_; req++)
			{
				best_split[req].resize(qexpand_size_alligned);
				uint32_t ncols_req = req_cols[req+1] - req_cols[req];
				::inaccel::Request request{"com.inaccel.xgboost.exact"};
				request.Arg((int)nrows_);
				request.Arg((int)ncols_req);
				request.Arg((int)qexpand.size());
				request.Arg((int)max_rows_);
				request.Arg(gpair_fpga);
				request.Arg(position_fpga_);
				request.Arg(dmat_fpga[req]);
				request.Arg(feat_valid_fpga_[req]);
				request.Arg(snode_stats_);
				request.Arg(snode_rg_);
				request.Arg(best_split[req]);
				request.Arg(param_.min_child_weight);
				request.Arg(param_.max_delta_step);
				request.Arg(param_.reg_alpha);
				request.Arg(param_.reg_lambda);
				requests.push_back(request);
			}
			for(uint32_t req = 0; req<nRequests_; req++)
			{
				::inaccel::Coral::SubmitAsync(requests[req]);
			}
			for(uint32_t req = 0; req<nRequests_; req++)
			{
				::inaccel::Coral::Await(requests[req]);
			}
			this->SyncBestSolution(qexpand, best_split, req_cols);
			for (int nid : qexpand) {
				NodeEntryInAccel &e = snode_[nid];
				if (e.best.loss_chg > kRtEps) {
					float left_leaf_weight =
							spliteval_->ComputeWeight(nid, GradStats(e.best.left_sum)) *
							param_.learning_rate;
					float right_leaf_weight =
							spliteval_->ComputeWeight(nid, GradStats(e.best.right_sum)) *
							param_.learning_rate;
					p_tree->ExpandNode(nid, e.best.SplitIndex(), e.best.split_value,
														 e.best.DefaultLeft(), e.weight, left_leaf_weight,
														 right_leaf_weight, e.best.loss_chg,
														 e.stats.sum_hess);
				} else {
					(*p_tree)[nid].SetLeaf(e.weight * param_.learning_rate);
				}
			}
		}
		void SyncBestSolution(const std::vector<int> &qexpand,
							  const std::vector<::inaccel::vector<SplitEntryInAccelRet>> &best_split,
							  const std::vector<uint32_t> &req_cols) {
			std::vector<SplitEntryInAccel> vec;
			for (int nid : qexpand) {
				for (uint32_t req = 0; req < nRequests_; req++) {
					this->snode_[nid].best.Update( SplitEntryInAccel(this->snode_[nid].stats,
													 best_split[req][node2workindex_[nid]],
													 req_cols[req]));
				}
				vec.push_back(this->snode_[nid].best);
			}
			// TODO(tqchen) lazy version
			// communicate best solution
			reducer_.Allreduce(dmlc::BeginPtr(vec), vec.size());
			// assign solution back
			for (size_t i = 0; i < qexpand.size(); ++i) {
				const int nid = qexpand[i];
				this->snode_[nid].best = vec[i];
			}
		}
		inline void ResetPosition(const std::vector<int> &qexpand,
									DMatrix* p_fmat,
									const RegTree& tree) {
			this->SetNonDefaultPosition(qexpand, p_fmat, tree);
			#pragma omp parallel for schedule(static)
			for (uint32_t ridx = 0; ridx < nrows_; ++ridx) {
				CHECK_LT(ridx, position_.size())
						<< "ridx exceed bound " << "ridx="<<	ridx << " pos=" << position_.size();
				const int nid = this->DecodePosition(ridx);
				if (tree[nid].IsLeaf()) {
					// mark finish when it is not a fresh leaf
					if (tree[nid].RightChild() == -1) {
						position_[ridx] = ~nid;
					}
				} else {
					// push to default branch
					if (tree[nid].DefaultLeft()) {
						this->SetEncodePosition(ridx, tree[nid].LeftChild());
					} else {
						this->SetEncodePosition(ridx, tree[nid].RightChild());
					}
				}
			}
		}
		void SetNonDefaultPosition(const std::vector<int> &qexpand, DMatrix *p_fmat,
									const RegTree &tree) {
			// step 2, classify the non-default data into right places
			std::vector<unsigned> fsplits;
			for (int nid : qexpand) {
				if (!tree[nid].IsLeaf()) {
					fsplits.push_back(tree[nid].SplitIndex());
				}
			}
			// get the candidate split index
			std::sort(fsplits.begin(), fsplits.end());
			fsplits.resize(std::unique(fsplits.begin(), fsplits.end()) - fsplits.begin());
			while (fsplits.size() != 0 && fsplits.back() >= ncols_) {
				fsplits.pop_back();
			}
			// bitmap is only word concurrent, set to bool first
			{
				auto ndata = static_cast<uint32_t>(this->position_.size());
				boolmap_.resize(ndata);
				#pragma omp parallel for schedule(static)
				for (uint32_t j = 0; j < ndata; ++j) {
						boolmap_[j] = 0;
				}
			}
			for (const auto &batch : p_fmat->GetSortedColumnBatches()) {
				for (auto fid : fsplits) {
					auto col = batch[fid];
					const auto ndata = static_cast<uint32_t>(col.size());
					#pragma omp parallel for schedule(static)
					for (uint32_t j = 0; j < ndata; ++j) {
						const uint32_t ridx = col[j].index;
						const float fvalue = col[j].fvalue;
						const int nid = this->DecodePosition(ridx);
						if (!tree[nid].IsLeaf() && tree[nid].SplitIndex() == fid) {
							if (fvalue < tree[nid].SplitCond()) {
								if (!tree[nid].DefaultLeft()) boolmap_[ridx] = 1;
							} else {
								if (tree[nid].DefaultLeft()) boolmap_[ridx] = 1;
							}
						}
					}
				}
			}

			bitmap_.InitFromBool(boolmap_);
			// communicate bitmap
			rabit::Allreduce<rabit::op::BitOR>(dmlc::BeginPtr(bitmap_.data), bitmap_.data.size());
			// get the new position
			#pragma omp parallel for schedule(static)
			for (uint32_t ridx = 0; ridx < nrows_; ++ridx) {
				const int nid = this->DecodePosition(ridx);
				if (bitmap_.Get(ridx)) {
					CHECK(!tree[nid].IsLeaf()) << "inconsistent reduce information";
					if (tree[nid].DefaultLeft()) {
						this->SetEncodePosition(ridx, tree[nid].RightChild());
					} else {
						this->SetEncodePosition(ridx, tree[nid].LeftChild());
					}
				}
			}
		}
		inline int DecodePosition(uint32_t ridx) const {
			const int pid = position_[ridx];
			return pid < 0 ? ~pid : pid;
		}
		inline void SetEncodePosition(uint32_t ridx, int nid) {
			if (position_[ridx] < 0) {
				position_[ridx] = ~nid;
			} else {
				position_[ridx] = nid;
			}
		}
		inline void UpdateQueueExpand(const RegTree& tree,
										const std::vector<int> &qexpand,
										std::vector<int>* p_newnodes) {
			p_newnodes->clear();
			for (int nid : qexpand) {
				if (!tree[ nid ].IsLeaf()) {
					p_newnodes->push_back(tree[nid].LeftChild());
					p_newnodes->push_back(tree[nid].RightChild());
				}
			}
		}
		inline void UpdatePosition(DMatrix* p_fmat, const RegTree &tree) {
			#pragma omp parallel for schedule(static)
			for (uint32_t ridx = 0; ridx < nrows_; ++ridx) {
				int nid = this->DecodePosition(ridx);
				while (tree[nid].IsDeleted()) {
					nid = tree[nid].Parent();
					CHECK_GE(nid, 0);
				}
				this->position_[ridx] = nid;
			}
		}
		inline const int* GetLeafPosition() const {
			return dmlc::BeginPtr(this->position_);
		}
	};
};

XGBOOST_REGISTER_TREE_UPDATER(DistFpgaMaker, "grow_fpga")
.describe("Distributed FPGA version of tree maker.")
.set_body([]() {
		return new DistFpgaMaker();
	});
}
}