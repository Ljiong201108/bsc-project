all:
	g++ -Wall -g -std=c++17 host.cpp -o app -I${XILINX_XRT}/include/ -L${XILINX_XRT}/lib/ -lOpenCL -lpthread -lrt -lstdc++
	v++ -c -t sw_emu --config zcu104.cfg -k vadd vadd.cpp -o vadd.xo 
	v++ -l -t sw_emu --config zcu104.cfg vadd.xo -o vadd.xclbin