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
from math import sqrt

class qa_mapper_bc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_mapper_bc(self):
        data = (1, 0, 1, 0,   1, 1, 0, 0)
        expected_result = (-1 - 1j, 1 - 1j, -1 + 1j, 1 + 1j)
        expected_result = [x / sqrt(2) for x in expected_result]
        src = blocks.vector_source_b_make(data)
        map = dab.mapper_bc_make(4)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, map, dst)
        self.tb.run()
        result_data = dst.data()
        #print expected_result
        #print result_data
        self.assertComplexTuplesAlmostEqual(expected_result, result_data, 6)


if __name__ == '__main__':
    gr_unittest.run(qa_mapper_bc, "qa_mapper_bc.xml")
