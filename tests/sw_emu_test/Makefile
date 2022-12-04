.PHONY: all run build clean host kernel link clean package

USER=liujio
ROOTFS=/home/$(USER)/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/$(USER)/bsc-project

COMP=$(WORKSPACE)/componnets
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host

HOST_INC_DIR=$(HOST)/common/include $(HOST)/security/include $(HOST)/test $(HOST)/data_compression/include /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2 /opt/xilinx/xrt/include
HOST_INC_FLG=$(foreach dir, $(HOST_INC_DIR), -I$(dir))
HOST_INC=$(foreach dir, $(HOST_INC_DIR), $(wildcard $(dir)/*.hpp))
HOST_FLG=-DFREE_RUNNING_KERNEL -DGZIP_MULTICORE_COMPRESS -D PARALLEL_BLOCK=1 -D GZIP_MODE=1 -Wall -g -std=c++1y -L${XILINX_XRT}/lib/ -lpthread -lrt -lstdc++ -lOpenCL # -DDECOMPRESS_MM
HOST_SRC=host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/security/src/*.cpp) $(wildcard $(HOST)/data_compression/src/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2/*.cpp)
HOST_OUTPUT=app

KNRL_INC=-I$(COMP)/common/include
KNRL_FLG=-t sw_emu --config config_kernel.cfg -g -D NUM_CORES=4 -D PARALLEL_BLOCK=1 -D MULTIPLE_BYTES=8 -D GZIP_MODE=1 -D LL_MODEL=true -DFREE_RUNNING_KERNEL

KNRL_SECURITY_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
KNRL_SECURITY_INC=-I$(COMP)/security/include -I$(KNRL)/security/include
KNRL_SECURITY=$(patsubst $(KNRL)/security/src/%.cpp, kernels/security/%.xo, $(KNRL_SECURITY_SRC))

KNRL_DATACOMPRESSION_SRC=$(wildcard $(KNRL)/data_compression/src/*.cpp)
KNRL_DATACOMPRESSION_INC=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include
KNRL_DATACOMPRESSION=$(patsubst $(KNRL)/data_compression/src/%.cpp, kernels/data_compression/%.xo, $(KNRL_DATACOMPRESSION_SRC))

KNRL_ALL=$(KNRL_SECURITY) $(KNRL_DATACOMPRESSION)
KNRL_INC+=$(KNRL_SECURITY_INC) $(KNRL_DATACOMPRESSION_INC)

LINK_INC=$(KNRL_INC)
LINK_FLG=-t sw_emu --config config_link.cfg -g -D NUM_CORES=4 -D PARALLEL_BLOCK=1 -D MULTIPLE_BYTES=8 -D GZIP_MODE=1 -D LL_MODEL=true -DFREE_RUNNING_KERNEL
LINK_OUTPUT=fpga.xclbin

PAKG_FLG=-t sw_emu --config config_package.cfg -g -D NUM_CORES=4 -D PARALLEL_BLOCK=1 -D MULTIPLE_BYTES=8 -D GZIP_MODE=1 -D LL_MODEL=true -DFREE_RUNNING_KERNEL

# XCL_EMULATION_MODE=sw_emu ./app
# package/launch_sw_emu.sh -forward-port 1440 22

package: $(LINK_OUTPUT) $(HOST_OUTPUT)
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh --package.sd_file sample.txt

build: $(HOST_OUTPUT) $(LINK_OUTPUT)

$(HOST_OUTPUT): $(HOST_SRC) $(HOST_INC)
	$(CXX) $(HOST_SRC) $(HOST_INC_FLG) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

kernels/security/%.xo: $(KNRL)/security/src/%.cpp
	v++ -c -k $(patsubst kernels/security/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/%.xo: $(KNRL)/data_compression/src/%.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

$(LINK_OUTPUT): $(KNRL_DATACOMPRESSION)
	v++ -l $^ -o $@ $(LINK_INC) $(LINK_FLG)

run: package
	package/launch_sw_emu.sh -forward-port 1440 22

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels