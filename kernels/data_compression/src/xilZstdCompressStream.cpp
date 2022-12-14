/*
 * (c) Copyright 2021 Xilinx, Inc. All rights reserved.
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
 *
 */

/**
 * @file zstd_compress_stream.cpp
 * @brief Source for Zstd compression kernel.
 *
 * This file is part of Vitis Data Compression Library.
 */

#include "xilZstdCompressStream.hpp"

extern "C" {
/**
 * @brief ZSTD compression kernel takes input data from axi stream and compresses it
 * into multiple frames having 1 block each and writes the compressed data to output axi stream.
 *
 *
 * @param inStream input raw data
 * @param outStream output compressed data
 */
void xilZstdCompressStream(hls::stream<ap_axiu<STREAM_IN_DWIDTH, 0, 0, 0> >& axiInStream,
                     hls::stream<ap_axiu<STREAM_OUT_DWIDTH, 0, 0, 0> >& axiOutStream) {
#ifdef FREE_RUNNING_KERNEL
#pragma HLS interface ap_ctrl_none port = return
#endif
    xf::compression::zstdCompressStreaming<STREAM_IN_DWIDTH, STREAM_OUT_DWIDTH, c_blockSize, c_windowSize,
                                           MIN_BLCK_SIZE>(axiInStream, axiOutStream);
}
}