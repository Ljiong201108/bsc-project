.PHONY: clean cleanall

include ../../make_environment.mk

OUTPUT=aes128Ofb
KERNELS=aes128OfbDec aes128OfbEnc

LINK_FLG+=--config config_link.cfg --optimize 2

KNRL_INC=-I$(COMP)/common/include
KNRL_INC+=-I$(COMP)/security/include -I$(KNRL)/security/include

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