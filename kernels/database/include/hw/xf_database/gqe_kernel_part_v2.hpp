/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _XF_DB_GQE_PART_H_
#define _XF_DB_GQE_PART_H_

/**
 * @file gqe_kernel_part_v2.hpp
 * @brief interface of GQE partition kernel.
 */

#include <ap_int.h>
#include <hls_stream.h>
#include "xf_database/types.hpp"

#define TPCH_INT_SZ 4
#define VEC_LEN 16
#define BURST_LEN 32
#define COL_NUM 8
#define CH_NUM 1
#define VEC_SCAN 16

const int HASHWH = 0;
const int HASHWL = 8;
const int PU = (1 << HASHWH);
// const int BK = (1 << HASHWL);

#ifndef __SYNTHESIS__
#include <iostream>
#endif

#define TEST_BUF_DEPTH (120000)

/**
 * @brief GQE partition kernel
 *
 * @param k_depth depth of each hash bucket in URAM
 * @param col_index index of input column
 * @param bit_num number of defined partition, log2(number of partition)
 *
 * @param buf_A input table buffer
 * @param buf_B output table buffer
 * @param buf_D configuration buffer
 *
 */
extern "C" void gqePart(const int k_depth,
                        const int col_index,
                        const int bit_num,
                        ap_uint<32 * VEC_SCAN> buf_A1[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A2[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A3[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A4[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A5[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A6[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A7[TEST_BUF_DEPTH],
                        ap_uint<32 * VEC_SCAN> buf_A8[TEST_BUF_DEPTH],
                        ap_uint<512> tin_meta[24],
                        ap_uint<512> tout_meta[24],
                        ap_uint<512> buf_B1[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B2[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B3[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B4[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B5[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B6[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B7[TEST_BUF_DEPTH],
                        ap_uint<512> buf_B8[TEST_BUF_DEPTH],

                        ap_uint<512> buf_D[9],
                        
                        ap_uint<72> *uram_bucket);

#endif // _XF_DB_GQE_PART_H_
