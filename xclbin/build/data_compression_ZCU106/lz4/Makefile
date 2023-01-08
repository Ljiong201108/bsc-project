.PHONY: clean cleanall

include ../../make_environment.mk

OUTPUT=lz4
KERNELS=xilLz4CompressMM xilLz4CompressStream xilLz4DecompressMM xilLz4DecompressStream xilCompressDatamover xilDecompressDatamover

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

xilLz4CompressMM.xo: $(KNRL)/data_compression/src/xilLz4CompressMM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilLz4CompressStream.xo: $(KNRL)/data_compression/src/xilLz4CompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilLz4DecompressMM.xo: $(KNRL)/data_compression/src/xilLz4DecompressMM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilLz4DecompressStream.xo: $(KNRL)/data_compression/src/xilLz4DecompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilCompressDatamover.xo: $(KNRL)/data_compression/src/xilCompressDatamover.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) 

xilDecompressDatamover.xo: $(KNRL)/data_compression/src/xilDecompressDatamover.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) 

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin *.info