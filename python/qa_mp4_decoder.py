#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2017 <+YOU OR YOUR COMPANY+>.
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

class qa_mp4_decoder (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        self.src01 = blocks.file_source_make(gr.sizeof_char, "debug/subch_energy_disp_undone_packed.dat")
        self.s2v = blocks.stream_to_vector_make(gr.sizeof_char, 4 * 84)
        self.mp4 = dab.mp4_decoder_make(14)
        self.dst = blocks.file_sink_make(gr.sizeof_char, "debug/superframes_firecode_checked.dat")

        self.tb.connect(self.src01, self.s2v, self.mp4, self.dst)
        self.tb.run ()
        pass


if __name__ == '__main__':
    gr_unittest.run(qa_mp4_decoder, "qa_mp4_decoder.xml")
