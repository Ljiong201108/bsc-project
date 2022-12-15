.PHONY: clean cleanall

include ../../../make_environment.mk

OUTPUT=aes192
KERNELS=aes192CbcDec aes192CbcEnc aes192CcmDec aes192CcmEnc aes192Cfb1Dec aes192Cfb1Enc aes192Cfb8Dec aes192Cfb8Enc aes192Cfb128Dec aes192Cfb128Enc aes192CtrDec aes192CtrEnc aes192EcbDec aes192EcbEnc aes192GcmDec aes192GcmEnc aes192OfbDec aes192OfbEnc

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