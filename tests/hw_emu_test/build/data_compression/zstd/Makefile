.PHONY: clean cleanall

include ../../../make_environment.mk

OUTPUT=zstd
KERNELS=xilZstdCompressDataMover xilZstdCompressStream xilZstdDecompressStream xilMM2S xilS2MM 

LINK_FLG+=--config config_link.cfg

LOG_DIR=log
KNRL_FLG+=--log_dir $(LOG_DIR)
LING_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
KNRL_FLG+=--report_dir $(REPORT_DIR)
LING_FLG+=--report_dir $(REPORT_DIR)

$(OUTPUT).xclbin: $(patsubst %, %.xo, $(KERNELS))
	v++ -l $^ -o $@ $(LINK_FLG)

xilZstdCompressDataMover.xo: $(KNRL)/data_compression/src/xilZstdCompressDataMover.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilZstdCompressStream.xo: $(KNRL)/data_compression/src/xilZstdCompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DFREE_RUNNING_KERNEL

xilZstdDecompressStream.xo: $(KNRL)/data_compression/src/xilZstdDecompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DFREE_RUNNING_KERNEL

xilMM2S.xo: $(KNRL)/data_compression/src/xilMM2S.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DMULTIPLE_BYTES=8

xilS2MM.xo: $(KNRL)/data_compression/src/xilS2MM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DMULTIPLE_BYTES=8

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin