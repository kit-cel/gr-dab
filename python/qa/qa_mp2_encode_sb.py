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
import os

class qa_mp2_encode_sb (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

# encodes a PCM sample, the resulting mp2 file can be verified by playing it wiht an audio player (e.g. VLC media player)
    def test_001_t (self):
        if os.path.exists("debug/PCM_piano_left.dat") and os.path.exists("debug/PCM_piano_right.dat"):
            self.src_left = blocks.file_source_make(gr.sizeof_short, "debug/PCM_piano_left.dat")
            self.src_right = blocks.file_source_make(gr.sizeof_short, "debug/PCM_piano_right.dat")
            self.mp2 = dab.mp2_encode_sb_make(14, 2, 48000)
            self.file_sink = blocks.file_sink_make(gr.sizeof_char, "debug/mp2_encoded.mp2")
            self.tb.connect(self.src_left, (self.mp2, 0), self.file_sink)
            self.tb.connect(self.src_right, (self.mp2, 1))
            self.tb.run ()
            # check data
        else:
            log = gr.logger("log")
            log.debug("debug file not found - skipped test")
            log.set_level("WARN")
        pass


if __name__ == '__main__':
    gr_unittest.run(qa_mp2_encode_sb, "qa_mp2_encode_sb.xml")
