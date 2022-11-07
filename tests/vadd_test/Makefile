all:
	export ROOTFS=/home/jiong/xilinx/xilinx-zynqmp-common-v2020.2
	$(CXX) -Wall -g -std=c++11 host.cpp -o app -I/usr/include/xrt -lOpenCL -lpthread -lrt -lstdc++
	v++ -c -t hw_emu --config zcu104.cfg -k vadd vadd.cpp -o vadd.xo 
	v++ -l -t hw_emu --config zcu104.cfg vadd.xo -o vadd.xclbin
	v++ -p -t hw_emu --config zcu104.cfg vadd.xclbin --package.out_dir package --package.rootfs ${ROOTFS}/rootfs.ext4 --package.sd_file ${ROOTFS}/Image --package.sd_file xrt.ini --package.sd_file app --package.sd_file vadd.xclbin