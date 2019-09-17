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
from . import dab_swig as dab
from gnuradio import audio
import os

class qa_mp4_encode_sb (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

# loopback test - manual check if encoded AAC frames are recognized and extracted properly of the decoder and audio is played correctly
    def test_001_t(self):
        if os.path.exists("debug/PCM_left.dat") and os.path.exists("debug/PCM_right.dat"):
            self.src_left = blocks.file_source_make(gr.sizeof_float, "debug/PCM_left.dat")
            self.src_right = blocks.file_source_make(gr.sizeof_float, "debug/PCM_right.dat")
            self.f2s_1 = blocks.float_to_short_make(1, 32767)
            self.f2s_2 = blocks.float_to_short_make(1, 32767)
            self.mp4_encode = dab.mp4_encode_sb_make(14, 2, 32000, 1)
            self.mp4_decode = dab.mp4_decode_bs_make(14)
            self.s2f_1 = blocks.short_to_float_make(1, 32767)
            self.s2f_2 = blocks.short_to_float_make(1, 32767)
            self.audio = audio.sink_make(32000)

            self.tb.connect(self.src_left, self.f2s_1, (self.mp4_encode, 0))
            self.tb.connect(self.src_right, self.f2s_2, (self.mp4_encode, 1))
            self.tb.connect(self.mp4_encode, self.mp4_decode, self.s2f_1, (self.audio, 0))
            self.tb.connect((self.mp4_decode, 1), self.s2f_2, (self.audio, 1))
            self.tb.run ()
            # check data
        else:
            log = gr.logger("log")
            log.debug("debug file not found - skipped test")
            log.set_level("WARN")
    pass



if __name__ == '__main__':
    gr_unittest.run(qa_mp4_encode_sb, "qa_mp4_encode_sb.xml")
