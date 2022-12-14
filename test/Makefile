.PHONY: get clean package runZCU runU50

include make_environment.mk

# PLATFORM=/opt/xilinx/platforms/xilinx_u50_gen3x16_xdma_201920_3/xilinx_u50_gen3x16_xdma_201920_3.xpfm
# XCL_EMULATION_MODE=$(TARGET) ./app
# package/launch_sw_emu.sh -forward-port 1440 22

# common
HOST_INC+=-I$(HOST)/common/include 

HOST_SRC+=$(HOST)/common/src/Application.cpp
HOST_SRC+=$(HOST)/common/src/BufferPointer.cpp
HOST_SRC+=$(HOST)/common/src/CommandQueuePointer.cpp
HOST_SRC+=$(HOST)/common/src/KernelPointer.cpp

# data_compression
HOST_INC+=-I$(COMP)/data_compression/include 
HOST_INC+=-I$(HOST)/data_compression/include 
HOST_INC+=-I$(HOST)/third_party/xxhash 
HOST_INC+=-I$(HOST)/security/include
HOST_INC+=-I$(APP)/data_compression/include

HOST_SRC+=$(HOST)/data_compression/src/Stream.cpp

HOST_SRC+=$(HOST)/data_compression/src/GzipZlib.cpp
HOST_SRC+=$(HOST)/data_compression/src/GzipZlibCompression.cpp
HOST_SRC+=$(HOST)/data_compression/src/GzipZlibDecompression.cpp
HOST_SRC+=$(APP)/data_compression/src/LibGzip.cpp

HOST_SRC+=$(HOST)/data_compression/src/Lz4.cpp
HOST_SRC+=$(HOST)/data_compression/src/Lz4Compression.cpp
HOST_SRC+=$(HOST)/data_compression/src/Lz4Decompression.cpp
# HOST_SRC+=$(APP)/data_compression/src/LibLz4.cpp

HOST_SRC+=$(HOST)/third_party/xxhash/xxhash.c

# database
# HOST_INC+=-I$(XILINX_VIVADO)/include
# HOST_INC+=-I$(XILINX_XRT)/include
# HOST_INC+=-I$(HOST)/database/include 
# HOST_INC+=-I$(KNRL)/database/include/sw 
# HOST_INC+=-I$(COMP)/database/include/sw
# HOST_INC+=-I$(COMP)/database/include/hw
# HOST_INC+=-I$(COMP)/utils/include
# HOST_INC+=-I$(TEST)/database

# HOST_SRC+=$(HOST)/database/src/gqe_aggr_config.cpp
# HOST_SRC+=$(HOST)/database/src/gqe_aggr.cpp
# HOST_SRC+=$(HOST)/database/src/gqe_join_config.cpp
# HOST_SRC+=$(HOST)/database/src/gqe_join.cpp
# HOST_SRC+=$(HOST)/database/src/gqe_ocl.cpp
# HOST_SRC+=$(HOST)/database/src/gqe_table.cpp

# test
# HOST_SRC+=$(TEST)/database/testJoin.cpp
# HOST_SRC+=$(TEST)/database/testAggr.cpp
# HOST_SRC+=$(TEST)/data_compression/testGzip.cpp
HOST_SRC+=$(TEST)/data_compression/testLz4.cpp

package: $(HOST_OUTPUT)
	v++ -p $(PAKG_FLG) $(FILE) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) $(patsubst %, --package.sd_file %, $(FILE)) --package.sd_file run_on_zcu.sh --package.sd_file sample.txt --package.sd_file test

$(HOST_OUTPUT): $(HOST_SRC)
	g++ $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

runZCU: package
	package/launch_$(TARGET).sh -forward-port 1440 22

runU50: $(HOST_OUTPUT)
	./app

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

get:
	scp -P 1440 root@127.0.0.1:/media/sd-mmcblk0p1/$(FILE) ./$(FILE)