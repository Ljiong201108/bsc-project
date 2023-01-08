USER=jiong
ROOTFS=/home/$(USER)/xilinx/xilinx-zynqmp-common-v2020.2
WORKSPACE=/home/$(USER)/bsc-project
PLATFORM?=/home/$(USER)/xilinx/platforms/xilinx_zcu104_base_202020_1/xilinx_zcu104_base_202020_1.xpfm
TARGET?=hw_emu

COMP=$(WORKSPACE)/components
KNRL=$(WORKSPACE)/kernels
HOST=$(WORKSPACE)/host
TEST=$(HOST)/test

KNRL_FLG=-t $(TARGET) -g --platform $(PLATFORM)

LINK_FLG=-t $(TARGET) -g --platform $(PLATFORM)

PAKG_FLG=-t $(TARGET) --config config_package.cfg --platform $(PLATFORM)

LOG_DIR=log
PAKG_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
PAKG_FLG+=--report_dir $(REPORT_DIR)