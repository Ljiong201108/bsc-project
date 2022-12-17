.PHONY: all run build clean host kernel link clean package

include make_environment.mk

# PLATFORM=/opt/xilinx/platforms/xilinx_u50_gen3x16_xdma_201920_3/xilinx_u50_gen3x16_xdma_201920_3.xpfm
# XCL_EMULATION_MODE=$(TARGET) ./app
# package/launch_sw_emu.sh -forward-port 1440 22

FILE=$(shell find $(TEST)/build -name "$(XCLBIN).xclbin")
FILES=$(shell find $(TEST)/build -name "*.xclbin")

package: $(HOST_OUTPUT)
	v++ -p $(PAKG_FLG) $(FILE) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) $(patsubst %, --package.sd_file %, $(FILE)) --package.sd_file run_on_zcu.sh --package.sd_file sample.txt --package.sd_file test

$(HOST_OUTPUT): $(HOST_SRC)
	$(CXX) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

runZCU: package
	package/launch_$(TARGET).sh -forward-port 1440 22

runU50: $(HOST_OUTPUT)
	cp /home/jiong/bsc-project/tests/test/build/data_compression/gzip_zlib/gzip_zlib.xclbin .
	cp /home/jiong/bsc-project/tests/test/build/data_compression/lz4/lz4.xclbin .
	cp /home/jiong/bsc-project/tests/test/build/data_compression/snappy/snappy.xclbin .
	cp /home/jiong/bsc-project/tests/test/build/data_compression/zstd/zstd.xclbin .
	./app
	rm -rf gzip_zlib.xclbin
	rm -rf lz4.xclbin
	rm -rf snappy.xclbin
	rm -rf zstd.xclbin

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

get:
	scp -P 1440 root@127.0.0.1:/media/sd-mmcblk0p1/$(FILE) ./$(FILE)