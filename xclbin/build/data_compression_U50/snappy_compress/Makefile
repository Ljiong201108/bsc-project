.PHONY: clean cleanall

include ../../make_environment.mk

OUTPUT=snappy_compress
KERNELS=xilSnappyCompressMM

KNRL_INC+=-I$(COMP)/security/include -I$(KNRL)/security/include
KNRL_INC+=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include

LINK_FLG+=--config config_link.cfg --optimize 2

LOG_DIR=log
KNRL_FLG+=--log_dir $(LOG_DIR)
LING_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
KNRL_FLG+=--report_dir $(REPORT_DIR)
LING_FLG+=--report_dir $(REPORT_DIR)

$(OUTPUT).xclbin: $(patsubst %, %.xo, $(KERNELS))
	v++ -l $^ -o $@ $(LINK_FLG)

xilSnappyCompressMM.xo: $(KNRL)/data_compression/src/xilSnappyCompressMM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilSnappyDecompressMM.xo: $(KNRL)/data_compression/src/xilSnappyDecompressMM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilSnappyCompressStream.xo: $(KNRL)/data_compression/src/xilSnappyCompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilSnappyDecompressStream.xo: $(KNRL)/data_compression/src/xilSnappyDecompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilCompressDatamover.xo: $(KNRL)/data_compression/src/xilCompressDatamover.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) 

xilDecompressDatamover.xo: $(KNRL)/data_compression/src/xilDecompressDatamover.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) 

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin *.info