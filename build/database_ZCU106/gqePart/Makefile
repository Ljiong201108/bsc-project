.PHONY: clean cleanall

include ../../make_environment.mk

OUTPUT=gqePart
KERNELS=gqePart

LINK_FLG+=--config config_link.cfg --optimize 2

LOG_DIR=log
KNRL_FLG+=--log_dir $(LOG_DIR)
LING_FLG+=--log_dir $(LOG_DIR)

REPORT_DIR=report
KNRL_FLG+=--report_dir $(REPORT_DIR)
LING_FLG+=--report_dir $(REPORT_DIR)

$(OUTPUT).xclbin: $(patsubst %, %.xo, $(KERNELS))
	v++ -l $^ -o $@ $(LINK_FLG)
	rm -rf _x .Xil log report *.log

KNRL_INC= 
# -I/home/jiong/Vitis_Libraries/database/L1/include/hw 
# -I/home/jiong/Vitis_Libraries/database/../utils/L1/include 
# -I/home/jiong/Vitis_Libraries/database/L2/include/hw 
# -I/home/jiong/Vitis_Libraries/database/L2/src/hw

KNRL_INC+=-I$(COMP)/database/include/hw 
KNRL_INC+=-I$(COMP)/utils/include
KNRL_INC+=-I$(KNRL)/database/include/hw 
KNRL_INC+=-I$(KNRL)/database/src/hw

gqePart.xo: $(KNRL)/database/src/hw/gqe_kernel_part_v2.cpp
	v++ -c -k $(patsubst %.xo, %, $@) $(KNRL_INC) $^ -o $@ $(KNRL_FLG)

clean:
	rm -rf _x .Xil log report *.log

cleanall: clean
	rm -rf *.link_summary *.compile_summary *.xo *.xclbin *.info