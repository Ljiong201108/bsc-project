.PHONY: all run build clean host kernel link

ROOTFS=/home/jiong/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/jiong/bsc-project

COMP=$(WORKSPACE)/componnets
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host

HOST_INC=-I$(HOST)/common/include -I$(HOST)/security/include
HOST_FLG=-Wall -g -std=c++17 -lOpenCL -lpthread -lrt -lstdc++ 
HOST_SRC=host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_OUTPUT=app

KNRL_INC=-I$(COMP)/security/include -I$(KNRL)/security/include -I$(COMP)/common/include
KNRL_FLG=-t hw_emu --config zcu106.cfg
KNRL_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
KNRL_OUTPUT=$(patsubst $(KNRL)/security/src/%.cpp, %.xo, $(KNRL_SRC))

LINK_INC=$(KNRL_INC)
LINK_FLG=$(KNRL_FLG)
LINK_OUTPUT=test.xclbin

PAKG_FLG=$(KNRL_FLG)

all: build run

host: $(HOST_OUTPUT)

kernel: $(XO)

link: $(LINK_OUTPUT)

$(HOST_OUTPUT): $(HOST_SRC)
	$(CXX) $(HOST_FLG) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT)

%.xo: $(KNRL)/security/src/%.cpp
	v++ -c $(KNRL_FLG) -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@

$(LINK_OUTPUT): $(KNRL_OUTPUT)
	v++ -l $(LINK_FLG) $(KNRLS_OUTPUTS) -o $(LINK_OUTPUT) $(LINK_INC)

build: host kernel link
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh

run: 
	./package/launch_hw_emu.sh -forward-port 1440 22

clean:
	rm -rf _x .Xil package app *.log *_summary *.xo *.xclbin *.info
	