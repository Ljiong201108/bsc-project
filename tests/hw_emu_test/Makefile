.PHONY: all run build clean host kernel link clean package

include make_environment.mk

# XCL_EMULATION_MODE=$(TARGET) ./app
# package/launch_sw_emu.sh -forward-port 1440 22

FILE=$(shell find $(TEST)/build -name "$(XCLBIN).xclbin")

package: $(HOST_OUTPUT)
	v++ -p $(PAKG_FLG) $(FILE) --package.out_dir package --package.rootfs $(ROOTFS)/rootfs.ext4 $(PAKG_SDFILE) --package.sd_file $(ROOTFS)/Image --package.sd_file xrt.ini --package.sd_file $(HOST_OUTPUT) $(patsubst %, --package.sd_file %, $(FILE)) --package.sd_file run_on_zcu.sh --package.sd_file sample.txt

$(HOST_OUTPUT): $(HOST_SRC)
	$(CXX) $(HOST_SRC) $(HOST_INC) $(HOST_LIB) -o $(HOST_OUTPUT) $(HOST_FLG)

# kernels/security/%.xo: $(KNRL)/security/src/%.cpp
# 	v++ -c -k $(patsubst kernels/security/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

# kernels/data_compression/%.xo: $(KNRL)/data_compression/src/%.cpp
# 	v++ -c -k $(patsubst kernels/data_compression/%.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

# $(LINK_OUTPUT): $(KNRL_DATACOMPRESSION)
# 	v++ -l $^ -o $@ $(LINK_INC) $(LINK_FLG)

run: package
	package/launch_$(TARGET).sh -forward-port 1440 22

clean:
	rm -rf .run *.csv _x .Xil package app *.log *_summary *.xo *.xclbin *.info kernels

get:
	scp -P 1440 root@127.0.0.1:/media/sd-mmcblk0p1/$(FILE) ./$(FILE)