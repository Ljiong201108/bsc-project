.PHONY: clean cleanall

include ../../make_environment.mk

OUTPUT=zstd
KERNELS=xilZstdDecompressStream xilMM2S xilS2MM xilZstdCompressDataMover xilZstdCompressStream

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

xilZstdCompressDataMover.xo: $(KNRL)/data_compression/src/xilZstdCompressDataMover.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

xilZstdCompressStream.xo: $(KNRL)/data_compression/src/xilZstdCompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DFREE_RUNNING_KERNEL

xilZstdDecompressStream.xo: $(KNRL)/data_compression/src/xilZstdDecompressStream.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DFREE_RUNNING_KERNEL -DINZSTD=1 -DINPUT_BYTES=4 -DOUTPUT_BYTES=8 -DZSTD_BLOCK_SIZE_KB=32

xilMM2S.xo: $(KNRL)/data_compression/src/xilMM2S.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DINZSTD=1 -DINPUT_BYTES=4 -DOUTPUT_BYTES=8 -DZSTD_BLOCK_SIZE_KB=32

xilS2MM.xo: $(KNRL)/data_compression/src/xilS2MM.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG) -DINZSTD=1 -DINPUT_BYTES=4 -DOUTPUT_BYTES=8 -DZSTD_BLOCK_SIZE_KB=32

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin *.info *.ltx