.PHONY: clean cleanall

include ../../../make_environment.mk

OUTPUT=aes256
KERNELS=aes256CbcDec aes256CbcEnc aes256CcmDec aes256CcmEnc aes256Cfb1Dec aes256Cfb1Enc aes256Cfb8Dec aes256Cfb8Enc aes256Cfb128Dec aes256Cfb128Enc aes256CtrDec aes256CtrEnc aes256EcbDec aes256EcbEnc aes256GcmDec aes256GcmEnc aes256OfbDec aes256OfbEnc

LINK_FLG+=--config config_link.cfg

LOG_DIR=log
KNRL_FLG+=--log_dir $(LOG_DIR)
LING_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
KNRL_FLG+=--report_dir $(REPORT_DIR)
LING_FLG+=--report_dir $(REPORT_DIR)

$(OUTPUT).xclbin: $(patsubst %, %.xo, $(KERNELS))
	v++ -l $^ -o $@ $(LINK_FLG)
	rm -rf _x .Xil log report *.log

%.xo: $(KNRL)/security/src/%.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin *.info