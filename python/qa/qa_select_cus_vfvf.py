#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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
import dab

class qa_select_cus_vfvf (gr_unittest.TestCase):
    """
    @brief QA for the select cus block

    This class implements a test bench to verify the corresponding C++ class.
    """

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t(self):
        vector01 = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
        expected_result = (1, 2, 3, 4, 9, 10, 11, 12)
        src = blocks.vector_source_b(vector01, True)
        b2f = blocks.char_to_float_make()
        s2v = blocks.stream_to_vector_make(gr.sizeof_float, 4)
        select_cus = dab.select_cus_vfvf_make(4, 2, 0, 1)
        v2s = blocks.vector_to_stream_make(gr.sizeof_float, 4)
        dst = blocks.vector_sink_f()
        self.tb.connect(src, b2f, s2v, select_cus, blocks.head_make(gr.sizeof_float*4, 2), v2s, dst)
        self.tb.run()
        result = dst.data()
        print result
        self.assertEqual(expected_result, result)

if __name__ == '__main__':
    gr_unittest.run(qa_select_cus_vfvf, "qa_select_cus_vfvf.xml")
