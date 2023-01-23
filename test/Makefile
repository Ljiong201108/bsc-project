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
HOST_SRC+=$(HOST)/common/src/Timer.cpp

# security
HOST_INC+=-I$(HOST)/security/include
HOST_INC+=-I$(APP)/security/include

HOST_SRC+=$(HOST)/security/src/security.cpp
HOST_SRC+=$(HOST)/security/src/Aes.cpp
HOST_SRC+=$(APP)/security/src/LibAes.cpp

HOST_SRC+=$(HOST)/test/security/testAes.cpp

# data_compression
# HOST_INC+=-I$(COMP)/data_compression/include 
# HOST_INC+=-I$(HOST)/data_compression/include 
# HOST_INC+=-I$(HOST)/third_party/xxhash 
# HOST_INC+=-I$(HOST)/security/include
# HOST_INC+=-I$(APP)/data_compression/include

# HOST_SRC+=$(HOST)/data_compression/src/Stream.cpp
# HOST_SRC+=$(HOST)/data_compression/src/Workshop.cpp

# HOST_SRC+=$(HOST)/data_compression/src/GzipZlib.cpp
# HOST_SRC+=$(HOST)/data_compression/src/GzipZlibCompress.cpp
# HOST_SRC+=$(HOST)/data_compression/src/GzipZlibDecompress.cpp
# HOST_SRC+=$(APP)/data_compression/src/LibGzip.cpp

# HOST_SRC+=$(HOST)/third_party/xxhash/xxhash.c
# HOST_SRC+=$(HOST)/data_compression/src/Lz4.cpp
# HOST_SRC+=$(HOST)/data_compression/src/Lz4Compress.cpp
# HOST_SRC+=$(HOST)/data_compression/src/Lz4Decompress.cpp
# HOST_SRC+=$(APP)/data_compression/src/LibLz4.cpp

# HOST_SRC+=$(HOST)/data_compression/src/Snappy.cpp
# HOST_SRC+=$(HOST)/data_compression/src/SnappyCompress.cpp
# HOST_SRC+=$(HOST)/data_compression/src/SnappyDecompress.cpp
# HOST_SRC+=$(APP)/data_compression/src/LibSnappy.cpp

# HOST_SRC+=$(HOST)/data_compression/src/Zstd.cpp
# HOST_SRC+=$(HOST)/data_compression/src/ZstdCompress.cpp
# HOST_SRC+=$(HOST)/data_compression/src/ZstdDecompress.cpp
# HOST_SRC+=$(APP)/data_compression/src/LibZstd.cpp

# HOST_SRC+=$(TEST)/data_compression/testGzip.cpp
# HOST_SRC+=$(TEST)/data_compression/testLz4.cpp
# HOST_SRC+=$(TEST)/data_compression/testSnappy.cpp
# HOST_SRC+=$(TEST)/data_compression/testZstd.cpp

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

# HOST_SRC+=$(TEST)/database/testJoin.cpp
# HOST_SRC+=$(TEST)/database/testAggr.cpp

# data_analytics
# HOST_INC+=-I$(HOST)/data_analytics/include
# HOST_INC+=-I$(HOST)/test/data_analytics
# HOST_INC+=-I$(COMP)/data_analytics/include/sw
# HOST_INC+=-I$(HOST)/data_analytics/ext/oniguruma/src
# HOST_INC+=-I$(XILINX_XRT)/include
# HOST_INC+=-I/home/jiong/Vitis_Libraries/data_analytics/L1/tests/text/regex_vm/re_compile/lib/include

# HOST_INC+=-L/home/jiong/Vitis_Libraries/data_analytics/L1/tests/text/regex_vm/re_compile -lxfcompile

# HOST_SRC+=$(HOST)/data_analytics/src/text/reEngine_config.cpp
# HOST_SRC+=$(HOST)/data_analytics/src/text/regex_engine.cpp
# HOST_SRC+=$(HOST)/test/data_analytics/testRegualrExpression.cpp
# HOST_SRC+=$(COMP)/data_analytics/src/sw/xf_re_compile.c

package: 
	rm -rf package
	v++ -p $(PAKG_FLG) aes128Cbc.xclbin --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file zcu --package.sd_file aes128Cbc.xclbin --package.sd_file run_on_zcu.sh --package.sd_file sample/sample.txt 

$(HOST_OUTPUT): $(HOST_SRC)
	$(CXX) $(HOST_SRC) $(HOST_LIB) $(HOST_INC) -o $(HOST_OUTPUT) $(HOST_FLG)

runZCU: package
	package/launch_$(TARGET).sh -forward-port 1440 22

u50: $(HOST_SRC)
	g++ $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o u50 $(HOST_FLG)

zcu: $(HOST_SRC)
	$(CXX) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o zcu $(HOST_FLG)

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

get:
	scp -P 1440 root@127.0.0.1:/media/sd-mmcblk0p1/$(FILE) ./$(FILE)