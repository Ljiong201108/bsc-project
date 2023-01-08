.PHONY: clean cleanall

include ../../make_environment.mk

OUTPUT=gzip_zlib
KERNELS=xilGzipZlibCompressMM xilGzipZlibDecompressMM xilGzipZlibDecompressStream xilMM2S xilS2MM

KNRL_INC+=-I$(COMP)/security/include -I$(KNRL)/security/include
KNRL_INC+=-I$(COMP)/data_compression/include -I$(KNRL)/data_compression/include

LINK_FLG+=--config config_link.cfg

LOG_DIR=log
KNRL_FLG+=--log_dir $(LOG_DIR)
LING_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
KNRL_FLG+=--report_dir $(REPORT_DIR)
LING_FLG+=--report_dir $(REPORT_DIR)

$(OUTPUT).xclbin: $(patsubst %, %.xo, $(KERNELS))
	v++ -l $^ -o $@ $(LINK_FLG)

xilGzipZlibCompressMM.xo: $(KNRL)/data_compression/src/xilGzipZlibCompressMM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilGzipZlibDecompressMM.xo: $(KNRL)/data_compression/src/xilGzipZlibDecompressMM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DLL_MODEL=true

xilGzipZlibDecompressStream.xo: $(KNRL)/data_compression/src/xilGzipZlibDecompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DMULTIPLE_BYTES=8 -DLL_MODEL=true -DFREE_RUNNING_KERNEL

xilMM2S.xo: $(KNRL)/data_compression/src/xilMM2S.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DMULTIPLE_BYTES=8

xilS2MM.xo: $(KNRL)/data_compression/src/xilS2MM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DMULTIPLE_BYTES=8

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin *.info