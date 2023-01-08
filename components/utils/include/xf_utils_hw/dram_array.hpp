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

/**
 * @file uram_array.hpp
 * @brief This file provides implementation of uram array helper class.
 *
 * This file is part of XF Common Utils Library.
 */

#ifndef XF_UTILS_HW_DRAM_ARRAY_H
#define XF_UTILS_HW_DRAM_ARRAY_H

#define blocks_(i, j, k) blocks[i*256+j*64+k]

#include <ap_int.h>

namespace xf {
namespace common {
namespace utils_hw {

/**
 * @tparam _WData  the width of every element.
 * @tparam _NData  the number of elements in the array.
 * @tparam _NCache the number of cache.
 */
template <int _WData, int _NData, int _NCache>
class DramArray {
   public:
    DramArray(ap_uint<72> *_blocks) :blocks(_blocks){
#pragma HLS inline
        initialize();
    }

   private:
    void initialize() {
        for (int i = 0; i < _NCache; i++) {
#pragma HLS unroll
            _index[i] = -1;
            _state[i] = 0;
        }
    }

   public:
    ~DramArray() {
    }

    int memSet(const ap_uint<_WData>& d);

    void write(int index, const ap_uint<_WData>& d);

    ap_uint<_WData> read(int index);

   private:
    static const int _elem_per_line;

    static const int _num_parallel_block;

    static const int _elem_per_block;

    static const int _num_uram_block;
    static const int _num_uram_x;
    static const int _num_uram_y;

    int _index[_NCache];

    ap_uint<_WData> _state[_NCache];

   public:
    //min 64*4*4096*72 bits
    ap_uint<72> *blocks;
};

// Const member variables

template <int _WData, int _NData, int _NCache>
const int DramArray<_WData, _NData, _NCache>::_elem_per_line = (72 / _WData);

template <int _WData, int _NData, int _NCache>
const int DramArray<_WData, _NData, _NCache>::_elem_per_block = (_elem_per_line * 4096);

template <int _WData, int _NData, int _NCache>
const int DramArray<_WData, _NData, _NCache>::_num_parallel_block = ((_WData + 71) / 72);

template <int _WData, int _NData, int _NCache>
const int DramArray<_WData, _NData, _NCache>::_num_uram_block = (details::need_num<_WData, _NData>::value_x *
                                                                 details::need_num<_WData, _NData>::value_y);

template <int _WData, int _NData, int _NCache>
const int DramArray<_WData, _NData, _NCache>::_num_uram_x = (details::need_num<_WData, _NData>::value_x);

template <int _WData, int _NData, int _NCache>
const int DramArray<_WData, _NData, _NCache>::_num_uram_y = (details::need_num<_WData, _NData>::value_y);

// Methods
template <int _WData, int _NData, int _NCache>
int DramArray<_WData, _NData, _NCache>::memSet(const ap_uint<_WData>& d) {
    if (_num_uram_block == 0) return 0;

    ap_uint<72> t;
l_init_value:
    for (int i = 0; i < 4096; i++) {
#pragma HLS PIPELINE II = 1
        for (int nx = 0; nx < _num_uram_x; nx++) {
#pragma HLS unroll
            for (int ny = 0; ny < _num_uram_y; ny++) {
#pragma HLS unroll
                if (_WData <= 72) {
                    for (int j = 0; j < _elem_per_line; j++) {
#pragma HLS unroll
                        t(j * _WData + _WData - 1, j * _WData) = d(_WData - 1, 0);
                    }
                    // blocks[nx][ny][i] = t;
                    blocks_(nx, ny, i) = t;
                } else {
                    int begin = ny * 72;
                    if (ny == (_num_parallel_block - 1))
                        // blocks[nx][ny][i] = d(_WData - 1, begin);
                        blocks_(nx, ny, i) = d(_WData - 1, begin)
                    else
                        // blocks[nx][ny][i] = d(begin + 71, begin);
                        blocks_(nx, ny, i) = d(begin + 71, begin)
                }
            }
        }
    }

// update d for cache
init_cache:
    for (int i = 0; i < _NCache; i++) {
#pragma HLS unroll
        _state[i] = d;
        _index[i] = 0;
    }

    return _num_uram_block;
}

template <int _WData, int _NData, int _NCache>
void DramArray<_WData, _NData, _NCache>::write(int index, const ap_uint<_WData>& d) {
#pragma HLS inline
    int div_block = 0, div_index = 0;
    int dec_block = 0, dec, begin;

Write_Inner:
    for (int i = 0; i < _num_parallel_block; i++) {
        if (_WData <= 72) {
            div_block = index / _elem_per_block;
            dec_block = index % _elem_per_block;
            div_index = dec_block / _elem_per_line;
            dec = dec_block % _elem_per_line;
            begin = dec * _WData;
            // ap_uint<72> tmp = blocks[div_block][0][div_index];
            ap_uint<72> tmp = blocks_(div_block, 0, div_index);
            tmp.range(begin + _WData - 1, begin) = d;
            // blocks[div_block][0][div_index] = tmp;
            blocks_(div_block, 0, div_index) = tmp;
        } else {
            div_block = index / 4096;
            dec_block = index % 4096;
            begin = i * 72;
            if (i == (_num_parallel_block - 1))
                // blocks[div_block][i][dec_block] = d.range(_WData - 1, begin);
                blocks_(div_block, i, dec_block) = d.range(_WData - 1, begin);
            else
                blocks_(div_block, i, dec_block) = d.range(begin + 71, begin);
                // blocks[div_block][i][dec_block] = d.range(begin + 71, begin);
        }
    }

Write_Cache:
    for (int i = _NCache - 1; i >= 1; i--) {
        _state[i] = _state[i - 1];
        _index[i] = _index[i - 1];
    }
    _state[0] = d;
    _index[0] = index;
}

template <int _WData, int _NData, int _NCache>
ap_uint<_WData> DramArray<_WData, _NData, _NCache>::read(int index) {
#pragma HLS inline
    ap_uint<_WData> value;
    int div_block = 0, div_index = 0;
    int dec_block = 0, dec, begin;

Read_Cache:
    for (int i = 0; i < _NCache; i++) {
        if (index == _index[i]) {
            value = _state[i];
            return value;
        }
    }

Read_Inner:
    for (int i = 0; i < _num_parallel_block; i++) {
        if (_WData <= 72) {
            div_block = index / _elem_per_block;
            dec_block = index % _elem_per_block;
            div_index = dec_block / _elem_per_line;
            dec = dec_block % _elem_per_line;
            begin = dec * _WData;
            ap_uint<72> tmp = blocks_(div_block, 0, div_index);
            // ap_uint<72> tmp = blocks[div_block][0][div_index];
            value = tmp.range(begin + _WData - 1, begin);
        } else {
            div_block = index / 4096;
            dec_block = index % 4096;
            begin = i * 72;

            if (i == (_num_parallel_block - 1))
                value.range(_WData - 1, begin) = blocks_(div_block, i, dec_block);
                // value.range(_WData - 1, begin) = blocks[div_block][i][dec_block];
            else {
                value.range(begin + 71, begin) = blocks_(div_block, i, dec_block);
                // value.range(begin + 71, begin) = blocks[div_block][i][dec_block];
            }
        }
    }
    return value;
}

} // utils_hw
} // common
} // xf

#endif
