#include <bits/stdc++.h>
#include "xf_data_analytics/text/helper.hpp"
#include "xf_data_analytics/text/reEngine_config.hpp"
#include "xf_data_analytics/text/regex_engine.hpp"
#include "Util.hpp"
#include "x_utils.hpp"

inline void hexdump(void *ptr, int buflen) {
    printf("Buffer Length = %d\n", buflen);
	unsigned char *buf = (unsigned char*)ptr;
	int i, j;
	for (i=0; i<buflen; i+=16) {
	printf("%06x: ", i);
	for (j=0; j<16; j++) 
		if (i+j < buflen)
		printf("%02x ", buf[i+j]);
		else
		printf("   ");
	printf(" ");
	for (j=0; j<16; j++) 
		if (i+j < buflen)
		printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
	printf("\n");
	}
}

void test(){
	// define your RE pattern
	std::string pattern = "test";
	// instantiate RegexEngine object
	xf::data_analytics::text::re::RegexEngine reInst(xclbinPath()+"/re_engine.xclbin");
	// corresponding error code
	xf::data_analytics::text::re::ErrCode err_code;
	// compile RE pattern
	err_code = reInst.compile(pattern);
	// error occurs
	if (err_code != 0) {
		exit(-1);
	}

	// enumerations
	enum {
		// Max number of messages in a section
		MAX_MSG_DEPTH = 250000000,
		// Max length of message in byte
		// XXX: actual max supported message length is defined in general configuration (general_config.hpp)
		// This one is used to split the input into several processing sections
		MAX_MSG_LEN = 65536,
		// Max number of lines in a single section
		MAX_LNM = 6000000,
		// 20 for 19 capturing groups at most
		MAX_OUT_DEPTH = MAX_LNM * 20
	};

	// utility for allocating the buffers
	xf::data_analytics::text::details::MM mm;
	// message buffer
	uint64_t* msg_buff = mm.aligned_alloc<uint64_t>(MAX_MSG_DEPTH);
	// offset address buffer
	uint32_t* offt_buff = mm.aligned_alloc<uint32_t>(MAX_LNM);
	// length of each message buffer
	uint16_t* len_buff = mm.aligned_alloc<uint16_t>(MAX_LNM);
	// output buffer
	uint32_t* out_buff = mm.aligned_alloc<uint32_t>(MAX_OUT_DEPTH);

	char simpleMessage[64]={"test12345678test"};
	std::memcpy(msg_buff, simpleMessage, 64);
	offt_buff[0]=0;
	len_buff[0]=16;

	reInst.match(1, msg_buff, offt_buff, len_buff, out_buff);

	hexdump(out_buff, 128);
}

int main(){
	test();
}