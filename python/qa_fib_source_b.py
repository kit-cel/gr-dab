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

class qa_fib_source_b (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    # manual check if transmitted data is interpreted properly
    # trivial fib_source with only one sub-channel
    def test_001_t(self):
        src = dab.fib_source_b_make(1,1,'Galaxy_News', 'Wasteland_Radio', 'Country_Mix', 0x09, [0], [8], [1])
        fib_unpacked_to_packed = blocks.unpacked_to_packed_bb(1, gr.GR_MSB_FIRST)
        s2v = blocks.stream_to_vector(gr.sizeof_char, 32)
        crc16 = dab.crc16_bb(32, 0x1021, 0xffff)
        fibsink = dab.fib_sink_vb()
        self.tb.connect(src, fib_unpacked_to_packed, blocks.head(gr.sizeof_char, 300), s2v, crc16, fibsink)
        self.tb.run()
        pass

    # multiple sub-channels
    def test_001_t(self):
        src = dab.fib_source_b_make(1,7,'Galaxy_News', 'Wasteland_Radio1Wasteland_Radio2Wasteland_Radio3Wasteland_Radio4Wasteland_Radio5Wasteland_Radio6Wasteland_Radio7', 'Country_Mix', 0x09, [0, 1, 2, 3, 3, 2, 1], [8, 2, 8, 8, 2, 1, 4], [1, 1, 0, 1, 1, 0, 1])
        fib_unpacked_to_packed = blocks.unpacked_to_packed_bb(1, gr.GR_MSB_FIRST)
        s2v = blocks.stream_to_vector(gr.sizeof_char, 32)
        crc16 = dab.crc16_bb(32, 0x1021, 0xffff)
        fibsink = dab.fib_sink_vb()
        self.tb.connect(src, fib_unpacked_to_packed, blocks.head(gr.sizeof_char, 3000), s2v, crc16, fibsink)
        self.tb.run()
        pass

if __name__ == '__main__':
    gr_unittest.run(qa_fib_source_b, "qa_fib_source_b.xml")
