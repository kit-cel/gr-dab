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
import math
from . import dab_swig as dab


class qa_qpsk_mapper_vbvc(gr_unittest.TestCase):
    """
    @brief QA for the qpsk mapper block

    This class implements a test bench to verify the corresponding C++ class.
    """

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_qpsk_mapper_vbvc(self):
        src_data = (31, 255)
        vector_length = 8
        SQRT2 = 1.0/math.sqrt(2)
        expected_result = (SQRT2 - SQRT2*1j, SQRT2 - SQRT2*1j, SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j,
                           -SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j)
        src = blocks.vector_source_b(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_char, vector_length/4)
        mapper = dab.qpsk_mapper_vbvc_make(vector_length)
        v2s = blocks.vector_to_stream(gr.sizeof_gr_complex, vector_length)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, s2v, mapper, v2s, dst)
        self.tb.run()
        result_data = dst.data()
        self.assertFloatTuplesAlmostEqual(expected_result, result_data, 6)

    def test_002_qpsk_mapper_vbvc(self):
        src_data = (0, 3)
        vector_length = 8
        SQRT2 = 1.0/math.sqrt(2)
        expected_result = (SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j,
                           SQRT2 + SQRT2*1j, SQRT2 - SQRT2*1j, SQRT2 - SQRT2*1j)
        src = blocks.vector_source_b(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_char, vector_length/4)
        mapper = dab.qpsk_mapper_vbvc_make(vector_length)
        v2s = blocks.vector_to_stream(gr.sizeof_gr_complex, vector_length)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, s2v, mapper, v2s, dst)
        self.tb.run()
        result_data = dst.data()
        self.assertFloatTuplesAlmostEqual(expected_result, result_data, 6)


if __name__ == '__main__':
    gr_unittest.run(qa_qpsk_mapper_vbvc, "qa_qpsk_mapper_vbvc.xml")
