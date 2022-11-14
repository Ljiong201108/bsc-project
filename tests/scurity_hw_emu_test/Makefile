.PHONY: all run build clean host kernel link clean_log

ROOTFS=/home/jiong/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/jiong/bsc-project

COMP=$(WORKSPACE)/componnets
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host

HOST_INC=-I$(HOST)/common/include -I$(HOST)/security/include -I./
HOST_FLG=-Wall -g -std=c++17 -lOpenCL -lpthread -lrt -lstdc++ 
HOST_SRC=$(TEST_NAME)/host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_OUTPUT=$(TEST_NAME)/app

KNRL_INC=-I$(COMP)/security/include -I$(KNRL)/security/include -I$(COMP)/common/include
KNRL_FLG=-t hw_emu --config zcu106.cfg
KNRL_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
KNRL_ALL=$(patsubst $(KNRL)/security/src/%.cpp, kernels/%.xo, $(KNRL_SRC))

LINK_INC=$(KNRL_INC)
LINK_FLG=$(KNRL_FLG)
LINK_ALL=$(patsubst $(KNRL)/security/src/%.cpp, test_%/test.xclbin, $(KNRL_SRC))
LINK_OUTPUT=$(TEST_NAME)/test.xclbin

PAKG_FLG=$(KNRL_FLG)

host: $(HOST_SRC)
	mkdir -p $(TEST_NAME)
	$(CXX) $(HOST_FLG) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT)

kernel: $(patsubst %, kernels/%.xo, $(KNRL_NEEDED))

kernel_all: $(KNRL_ALL)

link: $(LINK_OUTPUT)

link_all: $(LINK_ALL)

kernels/%.xo: $(KNRL)/security/src/%.cpp
	v++ -c $(KNRL_FLG) -k $(patsubst kernels/%.xo, %, $@) $(KNRL_INC) $^ -o $@

test_%/test.xclbin: kernels/%.xo 
	v++ -l $(LINK_FLG) $^ -o $@ $(LINK_INC)

package: $(LINK_OUTPUT) $(HOST_OUTPUT) $(patsubst test_%, kernels/%.xo, $(TEST_NAME))
	mkdir -p $(TEST_NAME)
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir $(TEST_NAME)/package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh

run: 
	$(TEST_NAME)/package/launch_hw_emu.sh -forward-port 1440 22

clean:
	rm -rf _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

clean_log:
	rm -rf *.log