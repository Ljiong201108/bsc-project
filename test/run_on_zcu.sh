export XILINX_XRT=/usr
export XCLBIN_PATH=/media/sd-mmcblk0p1
XCL_EMULATION_MODE=hw_emu ./zcu sample.txt sample.txt.cbc_enc sample.txt.cbc_enc.cbc_dec
