ifndef XILINX_SDX
$(error XILINX_SDX is not set)
endif

ifndef AWS_PLATFORM
$(error AWS_PLATFORM is not set)
endif

CLCC = xocc

BITSTREAM_NAME = xgboost_exact
PLATFORM := ${AWS_PLATFORM}

SRC_DIR := src
BUILD_DIR := build
BITSTREAM_DIR := bitstream

KERNEL_SRCS = $(notdir $(wildcard $(SRC_DIR)/*.cpp))
KERNEL_OBJECTS := $(KERNEL_SRCS:.cpp=.xo)

HOST_CFLAGS = -g -Wall -I${XILINX_SDX}/runtime/include/1_2
HOST_LFLAGS = -L${XILINX_SDX}/runtime/lib/x86_64 -lxilinxopencl

BANKS = --sp xgboost_exact_0_1.m_axi_gmem0:bank0 \
		--sp xgboost_exact_0_1.m_axi_gmem1:bank0 \
		--sp xgboost_exact_0_1.m_axi_gmem2:bank0 \
		--sp xgboost_exact_0_1.m_axi_gmem3:bank0 \
		--sp xgboost_exact_0_1.m_axi_gmem4:bank0 \
		--sp xgboost_exact_0_1.m_axi_gmem5:bank0 \
		--sp xgboost_exact_0_1.m_axi_gmem6:bank0 \
		--sp xgboost_exact_1_1.m_axi_gmem0:bank1 \
		--sp xgboost_exact_1_1.m_axi_gmem1:bank1 \
		--sp xgboost_exact_1_1.m_axi_gmem2:bank1 \
		--sp xgboost_exact_1_1.m_axi_gmem3:bank1 \
		--sp xgboost_exact_1_1.m_axi_gmem4:bank1 \
		--sp xgboost_exact_1_1.m_axi_gmem5:bank1 \
		--sp xgboost_exact_1_1.m_axi_gmem6:bank1

VIVADO_OPTS = --xp misc:enableGlobalHoldIter="True" \
			  --xp vivado_prop:run.impl_1.STEPS.ROUTE_DESIGN.ARGS.DIRECTIVE=NoTimingRelaxation 

all: $(KERNEL_OBJECTS:%.xo=$(BUILD_DIR)/%.xo)
	cd $(BUILD_DIR) && ${CLCC} -t hw --kernel_frequency "0:200" --link --platform ${PLATFORM} ${VIVADO_OPTS} \
		${BANKS} ${KERNEL_OBJECTS} -o ../$(BITSTREAM_DIR)/${BITSTREAM_NAME}.hw.xclbin && cd ../

$(BUILD_DIR)/%.xo: $(SRC_DIR)/%.cpp
	cd $(BUILD_DIR) && ${CLCC} -t hw --kernel_frequency "0:200" --platform ${PLATFORM} \
		--kernel $(notdir $(basename $<)) -c ../$< -o $(notdir $@) && cd ../

upload:
	cd $(BUILD_DIR) && $(AWS_FPGA_REPO_DIR)/SDAccel/tools/create_sdaccel_afi.sh \
		-xclbin=../$(BITSTREAM_DIR)/${BITSTREAM_NAME}.hw.xclbin -o=../$(BITSTREAM_DIR)/${BITSTREAM_NAME}.hw \
		-s3_bucket=$(BUCKET) -s3_dcp_key=${BITSTREAM_NAME}_dcp -s3_logs_key=${BITSTREAM_NAME}_logs && cd ../

clean:
	cd $(BUILD_DIR) && ${RM} -rf $(KERNEL_OBJECTS) emconfig.json *.log *.dir *.xml \
		*.dcp *.dat _sds iprepo *.tcl xilinx_aws-vu9p-f1_dynamic_5_0.hpfm .Xil sdaccel_* \
		system_estimate.xtxt _x top_sp.ltx *.txt && cd ../

clean_all: clean
	cd $(BITSTREAM_DIR) && ${RM} *.xclbin *.awsxclbin && cd ../

help:
	@echo "Compile .xclbin file"
	@echo "make all"
	@echo ""
	@echo "Create AFI "
	@echo "make upload"
	@echo ""
	@echo "Clean working directory"
	@echo "make clean"
