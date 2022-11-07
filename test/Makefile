.PHONY: all package run build clean

ROOTFS=/home/jiong/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/jiong/Vitis_Tests
SECURITY_SRC=$(WORKSPACE)/host/security/src

COMP=/home/jiong/Vitis_Tests/componnets
KNRL=/home/jiong/Vitis_Tests/kernels

HOST_INC=-I$(WORKSPACE)/host/common/include -I$(WORKSPACE)/host/security/include
HOST_LIB=-lOpenCL -lpthread -lrt -lstdc++ 
HOST_FLG=-Wall -g -std=c++17
HOST_SRC=host.cpp $(WORKSPACE)/host/common/src/* $(WORKSPACE)/host/security/src/*
HOST_OUTPUT=app

KNRL_INC=-I$(COMP)/security/include -I$(KNRL)/security/include -I$(COMP)/common/include
KNRL_LIB=
KNRL_FLG=-t hw_emu --config zcu106.cfg

KNRLS_OUTPUTS=

KNRL_3=desCbcEnc
KNRL_3_SRC=$(KNRL)/security/src/desCbcEnc.cpp
KNRL_3_OUTPUT=des_enc_cbc.xo
KNRLS_OUTPUTS+=$(KNRL_3_OUTPUT)

KNRL_4=desCbcDec
KNRL_4_SRC=$(KNRL)/security/src/desCbcDec.cpp
KNRL_4_OUTPUT=des_dec_cbc.xo
KNRLS_OUTPUTS+=$(KNRL_4_OUTPUT)

KNRL_1=desSingleBlockEnc
KNRL_1_SRC=$(KNRL)/security/src/desSingleBlockEnc.cpp
KNRL_1_OUTPUT=des_enc.xo
KNRLS_OUTPUTS+=$(KNRL_1_OUTPUT)

KNRL_2=desSingleBlockDec
KNRL_2_SRC=$(KNRL)/security/src/desSingleBlockDec.cpp
KNRL_2_OUTPUT=des_dec.xo
KNRLS_OUTPUTS+=$(KNRL_2_OUTPUT)

LINK_INC=$(KNRL_INC)
LINK_FLG=$(KNRL_FLG)
LINK_OUTPUT=test.xclbin

PAKG_FLG=$(KNRL_FLG)

all: build run

build: $(HOST_OUTPUT) $(KNRLS_OUTPUTS) $(LINK_OUTPUT) package

$(HOST_OUTPUT): $(HOST_SRC)
	$(CXX) $(HOST_FLG) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT)
 
$(KNRL_1_OUTPUT): $(KNRL_1_SRC)
	v++ -c $(KNRL_FLG) -k $(KNRL_1) $(KNRL_INC) $(KNRL_1_SRC) -o $(KNRL_1_OUTPUT)

$(KNRL_2_OUTPUT): $(KNRL_2_SRC)
	v++ -c $(KNRL_FLG) -k $(KNRL_2) $(KNRL_INC) $(KNRL_2_SRC) -o $(KNRL_2_OUTPUT)

$(KNRL_3_OUTPUT): $(KNRL_3_SRC)
	v++ -c $(KNRL_FLG) -k $(KNRL_3) $(KNRL_INC) $(KNRL_3_SRC) -o $(KNRL_3_OUTPUT)

$(KNRL_4_OUTPUT): $(KNRL_4_SRC)
	v++ -c $(KNRL_FLG) -k $(KNRL_4) $(KNRL_INC) $(KNRL_4_SRC) -o $(KNRL_4_OUTPUT)

$(LINK_OUTPUT): $(KNRLS_OUTPUTS)
	v++ -l $(LINK_FLG) $(KNRLS_OUTPUTS) -o $(LINK_OUTPUT) $(LINK_INC)

package: 
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh

run: 
	./package/launch_hw_emu.sh -forward-port 1440 22

clean:
	rm -rf _x .Xil package app *.log *_summary *.xo *.xclbin *.info