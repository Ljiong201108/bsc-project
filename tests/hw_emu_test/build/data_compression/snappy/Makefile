.PHONY: clean cleanall

include ../../../make_environment.mk

OUTPUT=snappy
KERNELS=xilSnappyCompressMM xilSnappyCompressStream xilSnappyDecompressMM xilSnappyDecompressStream xilCompressDatamover xilDecompressDatamover

LINK_FLG+=--config config_link.cfg

LOG_DIR=log
KNRL_FLG+=--log_dir $(LOG_DIR)
LING_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
KNRL_FLG+=--report_dir $(REPORT_DIR)
LING_FLG+=--report_dir $(REPORT_DIR)

$(OUTPUT).xclbin: $(patsubst %, %.xo, $(KERNELS))
	v++ -l $^ -o $@ $(LINK_FLG)
	clean

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
	rm -rf _x .Xil log report *.link_summary *.compile_summary *.log

cleanall:
	clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin