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
#from . import dab_swig as dab
from . import dab_swig as dab

class qa_conv_encoder_bb (gr_unittest.TestCase):
    """
    @brief QA for the convolutional encoder block

    This class implements a test bench to verify the corresponding C++ class.
    """

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t(self):
        """
        test of a 2 byte frame with reference data (calculated by hand)
        """
        data = (0x05, 0x00)
        expected_result = (0x00, 0x00, 0x0f, 0x62, 0xBF, 0x4D, 0x9F, 0x00, 0x00, 0x00, 0x00)
        src = blocks.vector_source_b(data)
        encoder = dab.conv_encoder_bb_make(2)
        sink = blocks.vector_sink_b()
        self.tb.connect(src, encoder, sink)
        self.tb.run()
        result = sink.data()
        #print result
        #print expected_result
        self.assertEqual(expected_result, result)


if __name__ == '__main__':
    gr_unittest.run(qa_conv_encoder_bb, "qa_conv_encoder_bb.xml")
