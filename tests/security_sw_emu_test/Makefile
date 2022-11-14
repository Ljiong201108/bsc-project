.PHONY: all run build clean host kernel link clean_logzcu104.cfg

ROOTFS=/home/jiong/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/jiong/bsc-project

COMP=$(WORKSPACE)/componnets
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host

HOST_INC=-I$(HOST)/common/include/ -I$(HOST)/security/include/ -I./ -I${XILINX_XRT}/include/ -I/home/jiong/bsc-project/host/test
HOST_FLG=-Wall -g -std=c++1y -L${XILINX_XRT}/lib/ -lpthread -lrt -lstdc++ -lOpenCL
HOST_SRC=host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_OUTPUT=app

KNRL_INC=-I$(COMP)/security/include -I$(KNRL)/security/include -I$(COMP)/common/include
KNRL_FLG=-t sw_emu --config config.cfg -g
KNRL_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
KNRL_ALL=$(patsubst $(KNRL)/security/src/%.cpp, kernels/%.xo, $(KNRL_SRC))
KNRL_NEEDED=aes128CbcEnc aes128CbcDec aes128CcmEnc aes128CcmDec aes128GcmEnc aes128GcmDec

LINK_INC=$(KNRL_INC)
LINK_FLG=$(KNRL_FLG)
LINK_ALL=$(patsubst $(KNRL)/security/src/%.cpp, test_%/test.xclbin, $(KNRL_SRC))
LINK_OUTPUT=test.xclbin

PAKG_FLG=$(KNRL_FLG)

# XCL_EMULATION_MODE=sw_emu ./app
# package/launch_sw_emu.sh -forward-port 1440 22

host: $(HOST_SRC)
	g++ $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

kernel: $(patsubst %, kernels/%.xo, $(KNRL_NEEDED))

link: $(LINK_OUTPUT)

kernels/%.xo: $(KNRL)/security/src/%.cpp
	v++ -c -k $(patsubst kernels/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

test.xclbin: $(patsubst %, kernels/%.xo, $(KNRL_NEEDED))
	v++ -l $^ -o $@ $(LINK_INC) $(LINK_FLG)

package: $(LINK_OUTPUT) $(HOST_OUTPUT) $(patsubst %, kernels/%.xo, $(KNRL_NEEDED))
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh

run: 
	XCL_EMULATION_MODE=sw_emu ./app

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

clean_log:
	rm -rf *.log