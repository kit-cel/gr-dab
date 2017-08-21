#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import dab_swig as dab

class qa_time_interleave_bb (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        vector01 =        (1, 2, 3, 4, 5, 6,    7, 8, 9, 10, 11, 12,   13, 14, 15, 16, 17, 18)
        expected_result = (1, 0, 3, 0, 5, 0,    7, 2, 9, 4, 11, 6,     13, 8, 15, 10, 17, 12)
        src = blocks.vector_source_b(vector01, True)
        s2v = blocks.stream_to_vector(gr.sizeof_char, 6)
        time_interleaver = dab.time_interleave_bb(6, [0, 1])
        v2s = blocks.vector_to_stream(gr.sizeof_char, 6)
        dst = blocks.vector_sink_b()
        self.tb.connect(src, s2v, time_interleaver, blocks.head_make(gr.sizeof_char*6, 3), v2s, dst)
        self.tb.run()
        result = dst.data()
        #print result
        self.assertEqual(expected_result, result)

    def test_002_t(self):
            vector01 =        (1, 2, 3, 4,     5, 6, 7, 8,     9, 10, 11, 12,  13, 14, 15, 16)
            expected_result = (1,0,0,0,        5, 0, 0, 4,     9,0,3,8,        13, 2, 7, 12)
            src = blocks.vector_source_b(vector01, True)
            s2v = blocks.stream_to_vector(gr.sizeof_char, 4)
            time_interleaver = dab.time_interleave_bb(4, [0, 3, 2, 1])
            v2s = blocks.vector_to_stream(gr.sizeof_char, 4)
            dst = blocks.vector_sink_b()
            self.tb.connect(src, s2v, time_interleaver, blocks.head_make(gr.sizeof_char * 4, 4), v2s, dst)
            self.tb.run()
            result = dst.data()
            #print result
            self.assertEqual(expected_result, result)

    def test_003_t(self):
        vector01 =          (1, 2, 3, 4, 5, 6, 7, 8,    9, 10, 11, 12, 13, 14, 15, 16,      17, 18, 19, 20, 21, 22, 23, 24,     25, 26, 27, 28, 29, 30, 31, 32)
        expected_result =   (0, 0, 3, 0, 0, 0, 7, 0,    0, 0, 11, 4, 0, 0, 15, 8,           1, 0, 19, 12, 5, 0, 23, 16,         9, 2, 27, 20, 13, 6, 31, 24)
        src = blocks.vector_source_b(vector01, True)
        s2v = blocks.stream_to_vector(gr.sizeof_char, 8)
        time_interleaver = dab.time_interleave_bb(8, [2, 3, 0, 1])
        v2s = blocks.vector_to_stream(gr.sizeof_char, 8)
        dst = blocks.vector_sink_b()
        self.tb.connect(src, s2v, time_interleaver, blocks.head_make(gr.sizeof_char * 8, 4), v2s, dst)
        self.tb.run()
        result = dst.data()
        #print result
        self.assertEqual(expected_result, result)

    def test_004_t(self):
        vector01 =          (1, 2, 3, 4, 5, 6,  7, 8, 9, 10, 11, 12,     13, 14, 15, 16, 17, 18,  19, 20, 21, 22, 23, 24,     25, 26, 27, 28, 29, 30,  31, 32, 33, 34, 35, 36)
        expected_result =   (0, 0, 3, 0, 0, 0, 7, 0,    0, 0, 11, 4, 0, 0, 15, 8,           1, 0, 19, 12, 5, 0, 23, 16,         9, 2, 27, 20, 13, 6, 31, 24)
        src = blocks.vector_source_b(vector01, True)
        s2v = blocks.stream_to_vector(gr.sizeof_char, 8)
        time_interleaver = dab.time_interleave_bb(8, [2, 3, 0, 1])
        v2s = blocks.vector_to_stream(gr.sizeof_char, 8)
        dst = blocks.vector_sink_b()
        self.tb.connect(src, s2v, time_interleaver, blocks.head_make(gr.sizeof_char * 8, 4), v2s, dst)
        self.tb.run()
        result = dst.data()
        #print result
        self.assertEqual(expected_result, result)

if __name__ == '__main__':
    gr_unittest.run(qa_time_interleave_bb, "qa_time_interleave_bb.xml")
