USER=jiong
ROOTFS=/home/$(USER)/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/$(USER)/bsc-project
PLATFORM?=/home/$(USER)/xilinx/platforms/xilinx_zcu104_base_202020_1/xilinx_zcu104_base_202020_1.xpfm
TARGET?=hw_emu

COMP=$(WORKSPACE)/components
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host
TEST=$(HOST)/test

HOST_INC= # -I$(HOST)/common/include -I$(HOST)/security/include -I$(HOST)/test -I$(HOST)/data_compression/include -I/opt/xilinx/xrt/include -I$(COMP)/data_compression/include -I$(HOST)/third_party/xxhash # /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2 
HOST_SRC= # host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/data_compression/src/*.cpp) $(HOST)/third_party/xxhash/xxhash.c # $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_FLG=-Wall -g -std=c++17 -L${XILINX_XRT}/lib/ -lpthread -lrt -lstdc++ -lOpenCL
HOST_OUTPUT=app

# KNRL_INC=-I$(COMP)/common/include
# KNRL_INC+=-I$(COMP)/security/include -I$(KNRL)/security/include
# KNRL_INC+=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include

KNRL_FLG=-t $(TARGET) -g --platform $(PLATFORM)

LINK_FLG=-t $(TARGET) -g --platform $(PLATFORM)

PAKG_FLG=-t $(TARGET) --config config_package.cfg --platform $(PLATFORM)

LOG_DIR=log
PAKG_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
PAKG_FLG+=--report_dir $(REPORT_DIR)