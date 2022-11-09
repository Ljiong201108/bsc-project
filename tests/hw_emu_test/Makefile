.PHONY: all run build clean host kernel link clean_log

ROOTFS=/home/jiong/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/jiong/bsc-project

COMP=$(WORKSPACE)/componnets
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host

HOST_INC=-I$(HOST)/common/include -I$(HOST)/security/include
HOST_FLG=-Wall -g -std=c++17 -lOpenCL -lpthread -lrt -lstdc++ 
HOST_SRC=host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_OUTPUT=$(APP_NAME)/app

KNRL_INC=-I$(COMP)/security/include -I$(KNRL)/security/include -I$(COMP)/common/include
KNRL_FLG=-t hw_emu --config zcu106.cfg
KNRL_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
KNRL_OUTPUT=$(patsubst $(KNRL)/security/src/%.cpp, kernels/%.xo, $(KNRL_SRC))

LINK_INC=$(KNRL_INC)
LINK_FLG=$(KNRL_FLG)
LINK_OUTPUT=$(APP_NAME)/test.xclbin

PAKG_FLG=$(KNRL_FLG)

APP_NAME=newApiTest
KNRL_NEED=$(patsubst $(KNRL)/security/src/%.cpp, %, $(KNRL_SRC))
XO=$(patsubst %, kernels/%.xo, $(KNRL_NEED))

all: build run

host: $(HOST_OUTPUT)

kernel: $(XO)

kernel_all: $(KNRL_OUTPUT)

link: $(LINK_OUTPUT)

$(HOST_OUTPUT): $(HOST_SRC)
	mkdir -p $(APP_NAME)
	$(CXX) $(HOST_FLG) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT)

kernels/%.xo: $(KNRL)/security/src/%.cpp
	v++ -c $(KNRL_FLG) -k $(patsubst kernels/%.xo, %, $@) $(KNRL_INC) $^ -o $@

$(LINK_OUTPUT): $(KNRL_OUTPUT)
	mkdir -p $(APP_NAME)
	v++ -l $(LINK_FLG) $(XO) -o $(LINK_OUTPUT) $(LINK_INC)

build: host kernel link
	mkdir -p $(APP_NAME)
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir $(APP_NAME)/package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh

run: 
	$(APP_NAME)/package/launch_hw_emu.sh -forward-port 1440 22

clean:
	rm -rf _x .Xil package app *.log *_summary *.xo *.xclbin *.info

clean_log:
	rm -rf *.log