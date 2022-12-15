.PHONY: clean cleanall

include ../../../make_environment.mk

OUTPUT=aes128
KERNELS=aes128CbcDec aes128CbcEnc aes128CcmDec aes128CcmEnc aes128Cfb1Dec aes128Cfb1Enc aes128Cfb8Dec aes128Cfb8Enc aes128Cfb128Dec aes128Cfb128Enc aes128CtrDec aes128CtrEnc aes128EcbDec aes128EcbEnc aes128GcmDec aes128GcmEnc aes128OfbDec aes128OfbEnc

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