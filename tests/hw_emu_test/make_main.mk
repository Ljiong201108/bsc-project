.PHONY: all run build clean host kernel link clean package

USER=liujio
ROOTFS=/home/$(USER)/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/$(USER)/bsc-project

COMP=$(WORKSPACE)/components
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host
TEST=$(WORKSPACE)/tests/hw_emu_test

HOST_INC=-I$(HOST)/common/include -I$(HOST)/security/include -I$(HOST)/test -I$(HOST)/data_compression/include -I/opt/xilinx/xrt/include -I$(COMP)/data_compression/include -I$(HOST)/third_party/xxhash # /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2 
HOST_FLG=-Wall -g -std=c++1y -L${XILINX_XRT}/lib/ -lpthread -lrt -lstdc++ -lOpenCL
HOST_SRC=host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/data_compression/src/*.cpp) $(HOST)/third_party/xxhash/xxhash.c # $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_OUTPUT=app

KNRL_INC=-I$(COMP)/common/include
KNRL_INC+=-I$(COMP)/security/include -I$(KNRL)/security/include
KNRL_INC+=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include

KNRL_FLG=-t hw_emu --config $(TEST)/config_kernel.cfg -g

HOST_INC+=$(KNRL_INC)

# KNRL_SECURITY_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
# KNRL_SECURITY_INC=-I$(COMP)/security/include -I$(KNRL)/security/include
# KNRL_SECURITY=$(patsubst $(KNRL)/security/src/%.cpp, kernels/security/%.xo, $(KNRL_SECURITY_SRC))

# KNRL_DATACOMPRESSION_SRC=$(wildcard $(KNRL)/data_compression/src/*.cpp)
# KNRL_DATACOMPRESSION_INC=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include
# KNRL_DATACOMPRESSION=$(patsubst $(KNRL)/data_compression/src/%.cpp, kernels/data_compression/%.xo, $(KNRL_DATACOMPRESSION_SRC))

# KNRL_ALL=$(KNRL_SECURITY) $(KNRL_DATACOMPRESSION)
# KNRL_INC+=$(KNRL_SECURITY_INC) $(KNRL_DATACOMPRESSION_INC)

# LINK_INC=$(KNRL_INC)
LINK_FLG=-t hw_emu --optimize quick -g
# LINK_OUTPUT=fpga.xclbin

PAKG_FLG=-t hw_emu --config $(TEST)/config_package.cfg

# XCL_EMULATION_MODE=hw_emu ./app
# package/launch_sw_emu.sh -forward-port 1440 22

package: $(LINK_OUTPUT) $(HOST_OUTPUT)
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh --package.sd_file sample.txt

build: $(HOST_OUTPUT) $(LINK_OUTPUT)

$(HOST_OUTPUT): $(HOST_SRC)
	$(CXX) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

kernels/security/%.xo: $(KNRL)/security/src/%.cpp
	v++ -c -k $(patsubst kernels/security/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/%.xo: $(KNRL)/data_compression/src/%.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

$(LINK_OUTPUT): $(KNRL_DATACOMPRESSION)
	v++ -l $^ -o $@ $(LINK_INC) $(LINK_FLG)

run: package
	package/launch_hw_emu.sh -forward-port 1440 22

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

get:
	scp -P 1440 root@127.0.0.1:/media/sd-mmcblk0p1/$(FILE) ./$(FILE)