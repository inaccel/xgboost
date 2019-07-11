#include <ap_int.h>
#include <ap_fixed.h>

#define MAX_ENTRY_NUM 65536
#define MAX_NODE_NUM 2048

//*************************************************
// type definitions
  typedef ap_fixed<32,16>                   fixed;
  typedef ap_uint<256>                      float8;
  typedef ap_uint<16>                       NID;
  typedef ap_uint<NID::width*8>             NID8;
  typedef ap_uint<8>                        bool8;
  typedef ap_uint<64>                       EntryP;
  typedef ap_uint<EntryP::width*8>          EntryP8;
  typedef ap_uint<256>                      SplitP;
  typedef ap_uint<SplitP::width*2>          SplitP2;
  typedef ap_uint<64>                       GSP;
  typedef ap_uint<GSP::width*8>             GSP8;
  typedef ap_uint<fixed::width*2>           GSFP;
  typedef ap_uint<GSFP::width+NID::width>   EIP;
  typedef ap_uint<GSFP::width+fixed::width> NIP;
//*************************************************
// basic type functions
  static float unpack_float(unsigned in)
  {
    #pragma HLS inline
    return *(float*)(&in);
  }
  static unsigned pack_float(float in)
  {
    #pragma HLS inline
    return *(unsigned*)(&in);
  }
//*************************************************
// Needed structs + struct functions
  struct Entry
  {
    unsigned index;
    float fvalue;
    Entry& from_EntryP( EntryP in)
    {
      #pragma HLS inline
      index = in.range(31, 0).to_uint();
      fvalue = unpack_float(in.range(63, 32).to_uint());
      return *this;
    }
  };
  struct GradStatsFixed
  {
    fixed sum_grad;
    fixed sum_hess;
    GradStatsFixed operator+(const GradStatsFixed& in)
    {
      #pragma HLS inline
      GradStatsFixed out;
      out.sum_grad = sum_grad + in.sum_grad;
      out.sum_hess = sum_hess + in.sum_hess;
      return out;
    }
    GradStatsFixed operator-(const GradStatsFixed& in)
    {
      #pragma HLS inline
      GradStatsFixed out;
      out.sum_grad = sum_grad - in.sum_grad;
      out.sum_hess = sum_hess - in.sum_hess;
      return out;
    }
    GSFP to_GSFP()
    {
      #pragma HLS inline
      GSFP tmp;
      tmp.range(fixed::width-1, 0) = sum_grad.range();
      tmp.range(fixed::width*2-1, fixed::width) = sum_hess.range();
      return tmp;
    }
    GradStatsFixed& from_GSFP(const GSFP& in)
    {
      #pragma HLS inline
      sum_grad.range() = in.range(fixed::width-1, 0);
      sum_hess.range() = in.range(fixed::width*2-1, fixed::width);
      return *this;
    }
    GSP to_GSP();
    GradStatsFixed& from_GSP(const GSP& in);
  };
  struct GradStats
  {
    float sum_grad;
    float sum_hess;
    GSP to_GSP()
    {
      #pragma HLS inline
      GSP tmp;
      tmp.range(31, 0) = pack_float(sum_grad);
      tmp.range(63, 32) = pack_float(sum_hess);
      return tmp;
    }
    GradStats& from_GSP(const GSP& in)
    {
      #pragma HLS inline
      sum_grad = unpack_float(in.range(31, 0));
      sum_hess = unpack_float(in.range(63, 32));
      return *this;
    }
  };
  GSP GradStatsFixed::to_GSP()
  {
    #pragma HLS inline
    GradStats tmp;
    tmp.sum_grad = sum_grad.to_float();
    tmp.sum_hess = sum_hess.to_float();
    return tmp.to_GSP();
  }
  GradStatsFixed& GradStatsFixed::from_GSP(const GSP& in)
  {
    #pragma HLS inline
    GradStats tmp;
    tmp.from_GSP(in);
    sum_grad = tmp.sum_grad;
    sum_hess = tmp.sum_hess;
    return *this;
  }
  struct EntryInfo
  {
    fixed gpair_grad;
    fixed gpair_hess;
    NID nid;
    EIP to_EIP()
    {
      #pragma HLS inline
      EIP tmp;
      tmp.range(fixed::width-1, 0) = gpair_grad.range();
      tmp.range(fixed::width*2-1, fixed::width) = gpair_hess.range();
      tmp.range(fixed::width*2+NID::width-1, fixed::width*2) = nid;
      return tmp;
    }
    EntryInfo& from_EIP(const EIP& in)
    {
      #pragma HLS inline
      gpair_grad.range() = in.range(fixed::width-1, 0);
      gpair_hess.range() = in.range(fixed::width*2-1, fixed::width);
      nid = in.range(fixed::width*2+NID::width-1, fixed::width*2);
      return *this;
    }
  };
  struct NodeInfo
  {
    fixed nstats_grad;
    fixed nstats_hess;
    fixed nrg;
    NIP to_NIP()
    {
      #pragma HLS inline
      NIP tmp;
      tmp.range(fixed::width-1, 0) = nstats_grad.range();
      tmp.range(fixed::width*2-1, fixed::width) = nstats_hess.range();
      tmp.range(fixed::width*3-1, fixed::width*2) = nrg.range();
      return tmp;
    }
    NodeInfo& from_NIP(const NIP& in)
    {
      #pragma HLS inline
      nstats_grad.range() = in.range(fixed::width-1, 0);
      nstats_hess.range() = in.range(fixed::width*2-1, fixed::width);
      nrg.range() = in.range(fixed::width*3-1, fixed::width*2);
      return *this;
    }
  };
  struct NodeTmpData
  {
    fixed accum_grad;
    fixed accum_hess;
    fixed prev_fvalue;
  };
  struct Split
  {
    fixed loss_chg;
    unsigned sindex;
    fixed fvalue;
    fixed left_child_grad;
    fixed left_child_hess;
    bool worse(Split &new_split)
    {
      #pragma HLS inline
      return new_split.loss_chg > loss_chg ||
          ((new_split.loss_chg == loss_chg) &
           ((new_split.sindex&0x7fffffff) <= (sindex&0x7fffffff)));
    }
    SplitP pack_split()
    {
      #pragma HLS inline
      SplitP split_out;
      split_out.range(31,0) = pack_float(loss_chg.to_float());
      split_out.range(63,32) = sindex;
      split_out.range(95,64) = pack_float(fvalue.to_float());
      split_out.range(127,96) = pack_float(left_child_grad.to_float());
      split_out.range(159,128) = pack_float(left_child_hess.to_float());
      split_out.range(255,160) = 0;
      return split_out;
    }
  }; 
//*************************************************
// calc functions
  static fixed calc_Gain(  GradStatsFixed     stats,
              fixed         param_min_child_weight,
              fixed         param_max_delta_step,
              fixed         param_reg_alpha,
              fixed         param_reg_lambda
            )
  {
    #pragma HLS inline
    fixed zero = 0.0f;
    fixed two = 2.0f;
    fixed new_hess = stats.sum_hess + param_reg_lambda;
    fixed new_grad_plus = stats.sum_grad + param_reg_alpha;
    fixed new_grad_minus = stats.sum_grad - param_reg_alpha;
    fixed new_grad;
    if (stats.sum_grad > param_reg_alpha)     new_grad = new_grad_minus;
    else if (stats.sum_grad < -param_reg_alpha) new_grad = new_grad_plus;
    else                     new_grad = 0.0f;
    fixed tmp_div = new_grad/new_hess;
    fixed gain_p1 = new_grad*tmp_div;
    fixed weight = -tmp_div;
    fixed weight_abs;
    if(weight >= zero) weight_abs = weight;
    else          weight_abs = -weight;
    fixed gain_p2 = param_reg_alpha * weight_abs;
    fixed new_weight;
    if (param_max_delta_step != zero && weight > param_max_delta_step)
      new_weight = param_max_delta_step;
    else if (param_max_delta_step != zero && weight < -param_max_delta_step)
      new_weight = -param_max_delta_step;
    else
      new_weight = weight;
    fixed gain_p3 = two * stats.sum_grad * new_weight;
    fixed score;
    if (param_max_delta_step == zero) score = gain_p1;
    else score = -(gain_p1 + gain_p2) + gain_p3;
    return score;
  }
  static fixed calc_Split_Gain(  GradStatsFixed   left,
                  GradStatsFixed   right,
                  fixed       param_min_child_weight,
                  fixed       param_max_delta_step,
                  fixed       param_reg_alpha,
                  fixed       param_reg_lambda
                )
  {
    #pragma HLS allocation instances=calc_Split_Gain limit=8 function
    #pragma HLS inline off
    fixed gainLeft = calc_Gain( left,
                  param_min_child_weight,
                  param_max_delta_step,
                  param_reg_alpha,
                  param_reg_lambda);
    fixed gainRight = calc_Gain( right,
                  param_min_child_weight,
                  param_max_delta_step,
                  param_reg_alpha,
                  param_reg_lambda);
    fixed gain = gainLeft + gainRight;
    return gain;
  }
  static SplitP keep_Best_Of_8(  unsigned       n,
                  Split       tmp_best_split_uram[8][MAX_NODE_NUM]
                )
  {
    #pragma HLS inline
    Split best_8[8];
    #pragma HLS array_partition variable=best_8 complete
    U_read_8: for(unsigned u=0; u<8; u++)
    {
      #pragma HLS unroll
      best_8[u] = tmp_best_split_uram[u][n];
    }
    Split best_4[4];
    #pragma HLS array_partition variable=best_4 complete
    U_best_4: for(unsigned u=0; u<4; u++)
    {
      #pragma HLS unroll
      if (best_8[u<<1].worse(best_8[(u<<1)+1]))
        best_4[u] = best_8[(u<<1)+1];
      else best_4[u] = best_8[u<<1];
    }
    Split best_2[2];
    #pragma HLS array_partition variable=best_2 complete
    U_best_2: for(unsigned u=0; u<2; u++)
    {
      #pragma HLS unroll
      
      if (best_4[u<<1].worse(best_4[(u<<1)+1]))
        best_2[u] = best_4[(u<<1)+1];
      else best_2[u] = best_4[u<<1];
    }
    Split split_out;
    if (best_2[0].worse(best_2[1]))
      split_out = best_2[1];
    else split_out = best_2[0];
    return split_out.pack_split();
  }
//*************************************************
// main
extern "C"
{
  void xgboost_exact_0( unsigned  entry_num,
                        unsigned  feature_num,
                        unsigned  node_num,
                        unsigned  entry_num_batch,
                        GSP8     *gpairs,
                        NID8    *node_idxs,
                        EntryP8  *entries,
                        bool8    *fvalid,
                        GSP8     *node_stats,
                        float8   *node_root_gain,
                        SplitP2  *best_splits,
                        float     param_min_child_weight,
                        float     param_max_delta_step,
                        float     param_reg_alpha,
                        float     param_reg_lambda
                      )
  {  
    #pragma HLS interface s_axilite port=entry_num bundle=control
    #pragma HLS interface s_axilite port=feature_num bundle=control
    #pragma HLS interface s_axilite port=node_num bundle=control
    #pragma HLS interface s_axilite port=entry_num_batch bundle=control

    #pragma HLS interface m_axi port=gpairs offset=slave bundle=gmem0
    #pragma HLS interface s_axilite port=gpairs bundle=control
    #pragma HLS interface m_axi port=node_idxs offset=slave bundle=gmem1
    #pragma HLS interface s_axilite port=node_idxs bundle=control
    #pragma HLS interface m_axi port=entries offset=slave bundle=gmem2
    #pragma HLS interface s_axilite port=entries bundle=control
    #pragma HLS interface m_axi port=fvalid offset=slave bundle=gmem3
    #pragma HLS interface s_axilite port=fvalid bundle=control
    #pragma HLS interface m_axi port=node_stats offset=slave bundle=gmem4
    #pragma HLS interface s_axilite port=node_stats bundle=control
    #pragma HLS interface m_axi port=node_root_gain offset=slave bundle=gmem5
    #pragma HLS interface s_axilite port=node_root_gain bundle=control
    #pragma HLS interface m_axi port=best_splits offset=slave bundle=gmem6
    #pragma HLS interface s_axilite port=best_splits bundle=control

    #pragma HLS interface s_axilite port=param_min_child_weight bundle=control
    #pragma HLS interface s_axilite port=param_max_delta_step bundle=control
    #pragma HLS interface s_axilite port=param_reg_alpha bundle=control
    #pragma HLS interface s_axilite port=param_reg_lambda bundle=control

    #pragma HLS interface s_axilite port=return bundle=control

    EIP local_EntryInfo_uram[4][MAX_ENTRY_NUM];
    #pragma HLS RESOURCE variable=local_EntryInfo_uram core=XPM_MEMORY uram
    #pragma HLS array_partition variable=local_EntryInfo_uram complete dim=1
    #pragma HLS array_partition variable=local_EntryInfo_uram cyclic factor=4 dim=2
    NIP local_NodeInfo_uram[4][MAX_NODE_NUM];
    #pragma HLS RESOURCE variable=local_NodeInfo_uram core=XPM_MEMORY uram
    #pragma HLS array_partition variable=local_NodeInfo_uram complete dim=1
    #pragma HLS array_partition variable=local_NodeInfo_uram cyclic factor=4 dim=2
    NodeTmpData tmp_ndata_uram[8][MAX_NODE_NUM];
    #pragma HLS array_partition variable=tmp_ndata_uram complete dim=1
    Split tmp_best_split_uram[8][MAX_NODE_NUM];
    #pragma HLS array_partition variable=tmp_best_loss_chg_uram complete dim=1
    unsigned entry_num_p8 = (entry_num>>3) + (((entry_num&0x7)>0)?1:0);
    unsigned entry_num_p16 = (entry_num>>4) + (((entry_num&0xf)>0)?1:0);
    unsigned node_num_p2 = (node_num>>1) + (((node_num&0x1)>0)?1:0);
    unsigned node_num_p8 = (node_num>>3) + (((node_num&0x7)>0)?1:0);
    unsigned node_num_p16 = (node_num>>4) + (((node_num&0xf)>0)?1:0);
    unsigned feature_num_p8 = (feature_num>>3) + (((feature_num&0x7)>0)?1:0);
    fixed zero = 0.0f;
    fixed half = 0.5f;
    fixed kRtEps;
    kRtEps.bit(0) = 1;

    fixed p_min_child_weight = param_min_child_weight;
    P_EntryInfo_Init: for(unsigned ep = 0; ep < entry_num_p8; ep++)
    {
      #pragma HLS loop_tripcount min=6250 max=6250
      #pragma HLS pipeline II=1
      GSP8 gpairs_in = gpairs[ep];
      NID8 node_idxs_in = node_idxs[ep];
      U_EntryInfo_Init: for (unsigned u = 0; u < 8; u++)
      {
        #pragma HLS unroll
        GradStatsFixed tmpGSF;
        tmpGSF.from_GSP(gpairs_in.range((u+1)*GSP::width-1, u*GSP::width));
        EntryInfo tmpEI;
        tmpEI.gpair_grad = tmpGSF.sum_grad;
        tmpEI.gpair_hess = tmpGSF.sum_hess;
        tmpEI.nid = node_idxs_in.range((u+1)*NID::width-1, u*NID::width);;
        local_EntryInfo_uram[0][(ep<<3)+u] = tmpEI.to_EIP();
        local_EntryInfo_uram[1][(ep<<3)+u] = tmpEI.to_EIP();
        local_EntryInfo_uram[2][(ep<<3)+u] = tmpEI.to_EIP();
        local_EntryInfo_uram[3][(ep<<3)+u] = tmpEI.to_EIP();
      }
    }
    P_NodeInfo_Init: for(unsigned np = 0; np < node_num_p8; np++)
    {
      #pragma HLS loop_tripcount min=20 max=20
      #pragma HLS pipeline II=1
      GSP8 nstats_in = node_stats[np];
      float8 nrg_in = node_root_gain[np];
      U_NodeInfo_Init: for (unsigned u = 0; u < 8; u++)
      {
        #pragma HLS unroll
        GradStatsFixed tmpGSF;
        tmpGSF.from_GSP(nstats_in.range((u+1)*GSP::width-1, u*GSP::width));
        NodeInfo tmpNI;
        tmpNI.nstats_grad = tmpGSF.sum_grad;
        tmpNI.nstats_hess = tmpGSF.sum_hess;
        tmpNI.nrg = unpack_float(nrg_in.range((u+1)*32 -1, u*32));
        local_NodeInfo_uram[0][(np<<3)+u] = tmpNI.to_NIP();
        local_NodeInfo_uram[1][(np<<3)+u] = tmpNI.to_NIP();
        local_NodeInfo_uram[2][(np<<3)+u] = tmpNI.to_NIP();
        local_NodeInfo_uram[3][(np<<3)+u] = tmpNI.to_NIP();
      }
    }
    P_clear_tmp_Brams: for(unsigned np = 0; np < node_num_p2; np++)
    {
      #pragma HLS loop_tripcount min=80 max=80
      #pragma HLS pipeline II=1
      U_clear_tmp_Brams: for(unsigned u=0; u<8; u++)
      {
        #pragma HLS unroll
        tmp_best_split_uram[u][np<<1].fvalue = 0;
        tmp_best_split_uram[u][np<<1].sindex = 0;
        tmp_best_split_uram[u][np<<1].loss_chg = 0;
        tmp_best_split_uram[u][np<<1].left_child_grad = 0;
        tmp_best_split_uram[u][np<<1].left_child_hess = 0;
        tmp_best_split_uram[u][(np<<1)+1].fvalue = 0;
        tmp_best_split_uram[u][(np<<1)+1].sindex = 0;
        tmp_best_split_uram[u][(np<<1)+1].loss_chg = 0;
        tmp_best_split_uram[u][(np<<1)+1].left_child_grad = 0;
        tmp_best_split_uram[u][(np<<1)+1].left_child_hess = 0;
      }
    }
    Feature_Loop: for(unsigned fp = 0; fp < feature_num_p8; fp++)
    {
      #pragma HLS loop_tripcount min=384 max=384
      bool8 new_feature_valid = fvalid[fp];
      if(new_feature_valid > 0)
      {
        bool curr_valid[8];
        #pragma HLS array_partition variable=curr_valid complete
        NID curr_nid[8];
        #pragma HLS array_partition variable=curr_nid complete
        NodeTmpData curr_ndata[8];
        #pragma HLS array_partition variable=curr_ndata complete
        bool curr_best_valid[8];
        #pragma HLS array_partition variable=curr_best_valid complete
        NID curr_best_nid[8];
        #pragma HLS array_partition variable=curr_best_nid complete
        Split curr_best_split[8];
        #pragma HLS array_partition variable=curr_best_split complete
        U_Init_Regs: for(unsigned u=0; u<8; u++)
        {
          #pragma HLS unroll
          curr_valid[u] = false;
          curr_nid[u] = -1;
          curr_ndata[u].accum_grad = 0;
          curr_ndata[u].accum_hess = 0;
          curr_ndata[u].prev_fvalue = 0;
          curr_best_valid[u] = false;
          curr_best_nid[u] = -1;
          curr_best_split[u].fvalue = 0;
          curr_best_split[u].sindex = 0;
          curr_best_split[u].loss_chg = 0;
          curr_best_split[u].left_child_grad = 0;
          curr_best_split[u].left_child_hess = 0;
        }
        P_Init_Brams: for(unsigned np = 0; np < node_num_p2; np++)
        {
          #pragma HLS loop_tripcount min=80 max=80
          #pragma HLS pipeline II=1
          U_Init_Brams: for(unsigned u=0; u<8; u++)
          {
            #pragma HLS unroll
            tmp_ndata_uram[u][np<<1].accum_grad = 0;
            tmp_ndata_uram[u][np<<1].accum_hess = 0;
            tmp_ndata_uram[u][np<<1].prev_fvalue = 0;
            tmp_ndata_uram[u][(np<<1)+1].accum_grad = 0;
            tmp_ndata_uram[u][(np<<1)+1].accum_hess = 0;
            tmp_ndata_uram[u][(np<<1)+1].prev_fvalue = 0;
          }
        }
        P_Entry_Loop_FW: for(unsigned e = 0; e < entry_num_batch; e++)
        {
          #pragma HLS loop_tripcount min=50000 max=50000
          #pragma HLS pipeline II=1
          #pragma HLS dependence variable=tmp_ndata_uram intra false
          #pragma HLS dependence variable=tmp_best_split_uram intra false
          EntryP8 entries_p_in = entries[fp*entry_num_batch + e];
          U_Entry_Loop_FW: for (unsigned u = 0; u < 8; u++)
          {
            #pragma HLS unroll
            Entry new_entry;
            new_entry.from_EntryP(entries_p_in.range((u+1)*EntryP::width-1, u*EntryP::width));
            bool new_entry_valid = (((fp<<3)+u) < feature_num) &&
                     (new_feature_valid.bit(u) == 1) && (new_entry.index < entry_num);
            fixed new_entry_fvalue = new_entry.fvalue;
            EntryInfo new_entry_info;
            if(new_entry_valid) new_entry_info.from_EIP(local_EntryInfo_uram[u>>1][new_entry.index]);
            bool new_nid_valid = (new_entry_info.nid < node_num) & new_entry_valid;
            bool nid_same = (curr_nid[u] == new_entry_info.nid);
            NodeInfo new_node_info;
            if(new_nid_valid) new_node_info.from_NIP(local_NodeInfo_uram[u>>1][new_entry_info.nid]);
            NodeTmpData tmp_ndata;
            if(nid_same) tmp_ndata = curr_ndata[u];
            else tmp_ndata = tmp_ndata_uram[u][new_entry_info.nid];
            if(curr_valid[u]& !(nid_same & new_nid_valid)) tmp_ndata_uram[u][curr_nid[u]] = curr_ndata[u];
            curr_nid[u] = new_entry_info.nid;
            curr_valid[u] = new_nid_valid;
            curr_ndata[u].accum_grad = tmp_ndata.accum_grad + new_entry_info.gpair_grad;
            curr_ndata[u].accum_hess = tmp_ndata.accum_hess + new_entry_info.gpair_hess;
            curr_ndata[u].prev_fvalue = new_entry_fvalue;
            Split new_split;
            new_split.fvalue = (tmp_ndata.prev_fvalue + new_entry_fvalue)*half;
            bool new_fvalue_valid = (tmp_ndata.prev_fvalue != new_entry_fvalue);
            GradStatsFixed new_stats;
            new_stats.sum_grad = tmp_ndata.accum_grad;
            new_stats.sum_hess = tmp_ndata.accum_hess;
            bool new_stats_valid = (new_stats.sum_hess != zero) &
                         (new_stats.sum_hess >= p_min_child_weight);
            GradStatsFixed tmp_c;
            tmp_c.sum_grad = new_node_info.nstats_grad - new_stats.sum_grad;
            tmp_c.sum_hess = new_node_info.nstats_hess - new_stats.sum_hess;
            bool tmp_c_valid = (tmp_c.sum_hess >= p_min_child_weight);
            new_split.left_child_grad = tmp_ndata.accum_grad;
            new_split.left_child_hess = tmp_ndata.accum_hess;
            new_split.sindex = (fp<<3)+u;
            new_split.loss_chg = calc_Split_Gain(  new_stats, tmp_c,
                                                   param_min_child_weight,
                                                   param_max_delta_step,
                                                   param_reg_alpha,
                                                   param_reg_lambda) - new_node_info.nrg;
            bool new_split_valid = new_nid_valid & new_fvalue_valid &
                                   new_stats_valid & tmp_c_valid;
            bool best_nid_same = (curr_best_nid[u] == new_entry_info.nid);
            Split tmp_split;
            if(best_nid_same) tmp_split = curr_best_split[u];
            else tmp_split = tmp_best_split_uram[u][new_entry_info.nid];
            if(curr_best_valid[u]& !(best_nid_same & new_nid_valid))
              tmp_best_split_uram[u][curr_best_nid[u]] = curr_best_split[u];
            curr_best_nid[u] = new_entry_info.nid;
            curr_best_valid[u] = new_nid_valid;
            if(new_split_valid & tmp_split.worse(new_split))
              curr_best_split[u] = new_split;
            else curr_best_split[u] = tmp_split;
          }
        }
        U_write_final_FW: for(unsigned u=0; u<8; u++)
        {
          #pragma HLS unroll
          if(curr_valid[u]) tmp_ndata_uram[u][curr_nid[u]] = curr_ndata[u];
          curr_nid[u] = -1;
          curr_valid[u] = false;
          if(curr_best_valid[u]) tmp_best_split_uram[u][curr_best_nid[u]] = curr_best_split[u];
          curr_best_nid[u] = -1;
          curr_best_valid[u] = false;
        }
        P_Node_Loop: for(unsigned n = 0; n < node_num; n++)
        {
          #pragma HLS loop_tripcount min=160 max=160
          #pragma HLS pipeline II=1
          NodeInfo new_node_info;
          new_node_info.from_NIP(local_NodeInfo_uram[0][n]);
          U_Node_Loop: for(unsigned u=0; u<8; u++)
          {
            #pragma HLS unroll
            NodeTmpData tmp_ndata = tmp_ndata_uram[u][n];
            fixed tmp_fvalue_abs;
            if(tmp_ndata.prev_fvalue >= 0) tmp_fvalue_abs = tmp_ndata.prev_fvalue;
            else tmp_fvalue_abs = -tmp_ndata.prev_fvalue;
            fixed gap = tmp_fvalue_abs + kRtEps;
            Split new_split;
            new_split.fvalue = tmp_ndata.prev_fvalue - gap;
            GradStatsFixed new_stats;
            new_stats.sum_grad = tmp_ndata.accum_grad;
            new_stats.sum_hess = tmp_ndata.accum_hess;
            bool new_stats_valid = (new_stats.sum_hess >= p_min_child_weight);
            GradStatsFixed tmp_c;
            tmp_c.sum_grad = new_node_info.nstats_grad - new_stats.sum_grad;
            tmp_c.sum_hess = new_node_info.nstats_hess - new_stats.sum_hess;
            bool tmp_c_valid = (tmp_c.sum_hess >= p_min_child_weight);
            new_split.left_child_grad = tmp_c.sum_grad;
            new_split.left_child_hess = tmp_c.sum_hess;
            new_split.sindex = ((fp<<3)+u) | 0x80000000;
            new_split.loss_chg = calc_Split_Gain(   tmp_c, new_stats,
                                                    param_min_child_weight,
                                                    param_max_delta_step,
                                                    param_reg_alpha,
                                                    param_reg_lambda) - new_node_info.nrg;
            bool new_split_valid = (((fp<<3)+u) < feature_num) & (new_feature_valid.bit(u) == 1) &
                                   new_stats_valid & tmp_c_valid;
            if(n>0) tmp_best_split_uram[u][n-1] = curr_best_split[u];
            curr_best_split[u] = tmp_best_split_uram[u][n];
            bool new_better = new_split_valid & curr_best_split[u].worse(new_split);
            if(new_better) curr_best_split[u] = new_split;
          }
        }
        U_write_best_final: for(unsigned u=0; u<8; u++)
        {
          #pragma HLS unroll
          tmp_best_split_uram[u][node_num-1] = curr_best_split[u];
        }
        P_Entry_Loop_BW: for(unsigned e = 0; e < entry_num_batch; e++)
        {
          #pragma HLS loop_tripcount min=50000 max=50000
          #pragma HLS pipeline II=1
          #pragma HLS dependence variable=tmp_ndata_uram intra false
          #pragma HLS dependence variable=tmp_best_split_uram intra false
          EntryP8 entries_p_in = entries[fp*entry_num_batch + e];
          U_Entry_Loop_BW: for (unsigned u = 0; u < 8; u++)
          {
            #pragma HLS unroll
            Entry new_entry;
            new_entry.from_EntryP(entries_p_in.range((u+1)*EntryP::width-1, u*EntryP::width));
            bool new_entry_valid = (((fp<<3)+u) < feature_num) &&
                     (new_feature_valid.bit(u) == 1) && (new_entry.index < entry_num);
            fixed new_entry_fvalue = new_entry.fvalue;
            EntryInfo new_entry_info;
            if(new_entry_valid) new_entry_info.from_EIP(local_EntryInfo_uram[u>>1][new_entry.index]);
            bool new_nid_valid = (new_entry_info.nid < node_num) & new_entry_valid;
            bool nid_same = (curr_nid[u] == new_entry_info.nid);
            NodeInfo new_node_info;
            if(new_nid_valid) new_node_info.from_NIP(local_NodeInfo_uram[u>>1][new_entry_info.nid]);
            NodeTmpData tmp_ndata;
            if(nid_same) tmp_ndata = curr_ndata[u];
            else tmp_ndata = tmp_ndata_uram[u][new_entry_info.nid];
            if(curr_valid[u]& !(nid_same & new_nid_valid)) tmp_ndata_uram[u][curr_nid[u]] = curr_ndata[u];
            curr_nid[u] = new_entry_info.nid;
            curr_valid[u] = new_nid_valid;
            curr_ndata[u].accum_grad = tmp_ndata.accum_grad - new_entry_info.gpair_grad;
            curr_ndata[u].accum_hess = tmp_ndata.accum_hess - new_entry_info.gpair_hess;
            curr_ndata[u].prev_fvalue = new_entry_fvalue;
            Split new_split;
            new_split.fvalue = (tmp_ndata.prev_fvalue + new_entry_fvalue)*half;
            bool new_fvalue_valid = (tmp_ndata.prev_fvalue != new_entry_fvalue);
            GradStatsFixed new_stats;
            new_stats.sum_grad = tmp_ndata.accum_grad - new_entry_info.gpair_grad;
            new_stats.sum_hess = tmp_ndata.accum_hess - new_entry_info.gpair_hess;
            bool new_stats_valid = (new_stats.sum_hess != zero) &
                         (new_stats.sum_hess >= p_min_child_weight);
            GradStatsFixed tmp_c;
            tmp_c.sum_grad = new_node_info.nstats_grad - new_stats.sum_grad;
            tmp_c.sum_hess = new_node_info.nstats_hess - new_stats.sum_hess;
            bool tmp_c_valid = (tmp_c.sum_hess >= p_min_child_weight);
            new_split.left_child_grad = tmp_c.sum_grad;
            new_split.left_child_hess = tmp_c.sum_hess;
            new_split.sindex = ((fp<<3)+u) | 0x80000000;
            new_split.loss_chg = calc_Split_Gain(  tmp_c, new_stats,
                                                   param_min_child_weight,
                                                   param_max_delta_step,
                                                   param_reg_alpha,
                                                   param_reg_lambda) - new_node_info.nrg;
            bool new_split_valid = new_nid_valid & new_fvalue_valid &
                         new_stats_valid & tmp_c_valid;
            bool best_nid_same = (curr_best_nid[u] == new_entry_info.nid);
            Split tmp_split;
            if(best_nid_same) tmp_split = curr_best_split[u];
            else tmp_split = tmp_best_split_uram[u][new_entry_info.nid];
            if(curr_best_valid[u]& !(best_nid_same & new_nid_valid))
              tmp_best_split_uram[u][curr_best_nid[u]] = curr_best_split[u];
            curr_best_nid[u] = new_entry_info.nid;
            curr_best_valid[u] = new_nid_valid;
            if(new_split_valid & tmp_split.worse(new_split))
              curr_best_split[u] = new_split;
            else curr_best_split[u] = tmp_split;
          }
        }
        U_write_final_BW: for(unsigned u=0; u<8; u++)
        {
          #pragma HLS unroll
          if(curr_valid[u]) tmp_ndata_uram[u][curr_nid[u]] = curr_ndata[u];
          curr_nid[u] = -1;
          curr_valid[u] = false;
          if(curr_best_valid[u]) tmp_best_split_uram[u][curr_best_nid[u]] = curr_best_split[u];
          curr_best_nid[u] = -1;
          curr_best_valid[u] = false;
        }
      }
    }
    P_Write_Back: for(unsigned np = 0; np < node_num_p2; np++)
    {
      #pragma HLS loop_tripcount min=80 max=80
      #pragma HLS pipeline II=1
      SplitP2 splits_out;
      splits_out.range(255,0) = keep_Best_Of_8( np<<1, tmp_best_split_uram);
      splits_out.range(511,256) = keep_Best_Of_8( (np<<1)+1, tmp_best_split_uram);
      best_splits[np] = splits_out;
    }
  }
}