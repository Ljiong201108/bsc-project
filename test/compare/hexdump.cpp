#include <bits/stdc++.h>

void hexdump(void *ptr, unsigned int buflen) {
	std::cout<<std::hex<<std::setfill('0');
	unsigned char *buf=(unsigned char*)ptr;
	unsigned int lenPerLine=64;
	unsigned int i, j;
	for (i=0;i<buflen;i+=lenPerLine) {
		std::cout<<std::setw(8)<<i<<" ";
		for (j=0;j<lenPerLine;j++){
			if (i+j<buflen) std::cout<<std::setw(2)<<(unsigned int)(unsigned char)buf[i+j]<<" ";
			else std::cout<<"   ";
		}
		std::cout<<" ";
		for(j=0;j<lenPerLine;j++){
			if (i+j<buflen) std::cout<<(char)(isprint(buf[i+j]) ? buf[i+j] : '.');
		}
		std::cout<<std::endl;
	}
	std::cout<<std::dec;
}

int main(int argc, char** argv){
	const uint64_t bufSize=64*1024*1024;
	std::vector<char> buf(bufSize);
    std::ifstream ifile;

	ifile.open(argv[1], std::ios::binary);
	int fileSize=4096;

	ifile.read(buf.data(), fileSize);
	freopen(argv[2], "w", stdout);
	hexdump(buf.data(), fileSize);
}