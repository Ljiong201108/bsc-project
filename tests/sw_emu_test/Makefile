.PHONY: all run build clean host kernel link clean package

USER=liujio
ROOTFS=/home/$(USER)/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/$(USER)/bsc-project

COMP=$(WORKSPACE)/components
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host

HOST_INC_DIR=$(HOST)/common/include $(HOST)/security/include $(HOST)/test $(HOST)/data_compression/include /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2 /opt/xilinx/xrt/include $(COMP)/data_compression/include $(HOST)/third_party/xxhash
HOST_INC_FLG=$(foreach dir, $(HOST_INC_DIR), -I$(dir))
HOST_INC=$(foreach dir, $(HOST_INC_DIR), $(wildcard $(dir)/*.hpp))
HOST_FLG=-Wall -g -std=c++1y -L${XILINX_XRT}/lib/ -lpthread -lrt -lstdc++ -lOpenCL # -DSNAPPY_STREAM -DXILINX
HOST_SRC=host.cpp $(wildcard $(HOST)/common/src/*.cpp) $(wildcard $(HOST)/data_compression/src/*.cpp) # $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/compress/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/cmdparser/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/logger/*.cpp) $(wildcard /home/$(USER)/bsc-project/backup/data_compression/common/libs/xcl2/*.cpp) $(wildcard $(HOST)/security/src/*.cpp)
HOST_OUTPUT=app

HOST_SRC+=$(HOST)/third_party/xxhash/xxhash.c
HOST_INC_FLG+=$(KNRL_INC)
# HOST_FLG+=-DXILINX

KNRL_INC=-I$(COMP)/common/include
KNRL_FLG=-t sw_emu --config config_kernel.cfg -g

KNRL_SECURITY_SRC=$(wildcard $(KNRL)/security/src/*.cpp)
KNRL_SECURITY_INC=-I$(COMP)/security/include -I$(KNRL)/security/include
KNRL_SECURITY=$(patsubst $(KNRL)/security/src/%.cpp, kernels/security/%.xo, $(KNRL_SECURITY_SRC))

KNRL_DATACOMPRESSION_SRC=$(wildcard $(KNRL)/data_compression/src/*.cpp)
KNRL_DATACOMPRESSION_INC=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include
KNRL_DATACOMPRESSION=$(patsubst $(KNRL)/data_compression/src/%.cpp, kernels/data_compression/%.xo, $(KNRL_DATACOMPRESSION_SRC))

KNRL_ALL=$(KNRL_SECURITY) $(KNRL_DATACOMPRESSION)
KNRL_INC+=$(KNRL_SECURITY_INC) $(KNRL_DATACOMPRESSION_INC)

LINK_INC=# $(KNRL_DATACOMPRESSION_INC)
LINK_FLG=-t sw_emu --config config_link.cfg -g
LINK_OUTPUT=fpga.xclbin

PAKG_FLG=-t sw_emu --config config_package.cfg -g

# XCL_EMULATION_MODE=sw_emu ./app
# package/launch_sw_emu.sh -forward-port 1440 22

package: $(LINK_OUTPUT) $(HOST_OUTPUT)
	v++ -p $(PAKG_FLG) $(LINK_OUTPUT) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) --package.sd_file $(LINK_OUTPUT) --package.sd_file run_on_zcu.sh --package.sd_file sample.txt

build: $(LINK_OUTPUT) $(HOST_OUTPUT)

$(HOST_OUTPUT): $(HOST_SRC) $(HOST_INC)
	$(CXX) $(HOST_SRC) $(HOST_INC_FLG) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

kernels/security/%.xo: $(KNRL)/security/src/%.cpp
	v++ -c -k $(patsubst kernels/security/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/xilGzipZlibCompressMM.xo: $(KNRL)/data_compression/src/xilGzipZlibCompressMM.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DNUM_CORES=4 -DPARALLEL_BLOCK=1 -DMULTIPLE_BYTES=8 -DGZIP_MODE=1 -DLL_MODEL=true -DFREE_RUNNING_KERNEL

kernels/data_compression/xilGzipZlibDecompressMM.xo: $(KNRL)/data_compression/src/xilGzipZlibDecompressMM.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DNUM_CORES=4 -DPARALLEL_BLOCK=1 -DMULTIPLE_BYTES=8 -DGZIP_MODE=1 -DLL_MODEL=true -DFREE_RUNNING_KERNEL

kernels/data_compression/xilGzipZlibDecompressStream.xo: $(KNRL)/data_compression/src/xilGzipZlibDecompressStream.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DNUM_CORES=4 -DPARALLEL_BLOCK=1 -DMULTIPLE_BYTES=8 -DGZIP_MODE=1 -DLL_MODEL=true -DFREE_RUNNING_KERNEL

kernels/data_compression/xilGzipZlibMM2S.xo: $(KNRL)/data_compression/src/xilGzipZlibMM2S.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DNUM_CORES=4 -DPARALLEL_BLOCK=1 -DMULTIPLE_BYTES=8 -DGZIP_MODE=1 -DLL_MODEL=true -DFREE_RUNNING_KERNEL

kernels/data_compression/xilGzipZlibS2MM.xo: $(KNRL)/data_compression/src/xilGzipZlibS2MM.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DNUM_CORES=4 -DPARALLEL_BLOCK=1 -DMULTIPLE_BYTES=8 -DGZIP_MODE=1 -DLL_MODEL=true -DFREE_RUNNING_KERNEL

kernels/data_compression/xilSnappyCompressMM.xo: $(KNRL)/data_compression/src/xilSnappyCompressMM.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/xilSnappyDecompressMM.xo: $(KNRL)/data_compression/src/xilSnappyDecompressMM.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/xilSnappyCompressStream.xo: $(KNRL)/data_compression/src/xilSnappyCompressStream.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/xilSnappyDecompressStream.xo: $(KNRL)/data_compression/src/xilSnappyDecompressStream.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/xilCompressDatamover.xo: $(KNRL)/data_compression/src/xilCompressDatamover.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/xilDecompressDatamover.xo: $(KNRL)/data_compression/src/xilDecompressDatamover.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

kernels/data_compression/%.xo: $(KNRL)/data_compression/src/%.cpp
	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

$(LINK_OUTPUT): $(KNRL_DATACOMPRESSION)
	v++ -l $^ -o $@ $(LINK_INC) $(LINK_FLG)

run: package
	package/launch_sw_emu.sh -forward-port 1440 22

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

get:
	scp -P 1440 root@127.0.0.1:/media/sd-mmcblk0p1/$(FILE) ./$(FILE)