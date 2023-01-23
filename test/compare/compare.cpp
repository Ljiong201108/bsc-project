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
	std::vector<char> buf1(bufSize), buf2(bufSize);
    std::ifstream ifile1, ifile2;

	ifile1.open(argv[1], std::ios::binary);
	ifile2.open(argv[2], std::ios::binary);

	ifile1.seekg(0, std::ios_base::end);
	uint64_t fileSize1=ifile1.tellg();
	ifile1.seekg(0, std::ios_base::beg);

	ifile2.seekg(0, std::ios_base::end);
	uint64_t fileSize2=ifile2.tellg();
	ifile2.seekg(0, std::ios_base::beg);

	if(fileSize1!=fileSize2){
		std::cout<<"file sizes are different!"<<std::endl;
		std::cout<<fileSize1<<" "<<fileSize2<<std::endl;
		return 0;
	}

	int i;
	for(i=0;i<fileSize1;i+=bufSize){
		ifile1.read(buf1.data(), bufSize);
		ifile2.read(buf2.data(), bufSize);

		for(int j=0;j<bufSize;j++){
			if(buf1[j]!=buf2[j]){
				std::cout<<"the "<<i+j<<"-th byte is different"<<std::endl;
				hexdump(buf1.data(), 512);
				std::cout<<std::endl;
				hexdump(buf2.data(), 512);
				return 0;
			}
		}
	}

	if(i<fileSize1){
		ifile1.read(buf1.data(), fileSize1-i);
		ifile2.read(buf2.data(), fileSize1-i);
		for(int j=0;j<fileSize1-i;j++){
			if(buf1[j]!=buf2[j]){
				std::cout<<"the "<<i+j<<"-th byte is different"<<std::endl;
				return 0;
			}
		}
	}

	std::cout<<"files are the same"<<std::endl;
}