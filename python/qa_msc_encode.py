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
from gnuradio import audio
import os
import dab_swig as dab


class qa_msc_encode(gr_unittest.TestCase):
    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

# manual loopback test: input = DAB+ superframes (mp4 and Reed-Solomon encoded but not channel encoded), output = PCM audio to sound card
    def test_001_t (self):
        log = gr.logger("log")
        if os.path.exists("debug/rs_encoded.dat") and os.path.exists("debug/rs_encoded.dat"):
            self.dp = dab.parameters.dab_parameters(1, 208.064e6, True)

            # sources
            self.fib_src = dab.fib_source_b_make(1, 1, 'Galaxy_News', 'Wasteland_Radio', 'Country_Mix01', 0x09, [2], [14])
            self.fib_pack = blocks.unpacked_to_packed_bb_make(1, gr.GR_MSB_FIRST)
            self.subch_src01 = blocks.file_source_make(gr.sizeof_char, "debug/rs_encoded.dat", True)
            self.subch_src02 = blocks.file_source_make(gr.sizeof_char, "debug/rs_encoded_2.dat", True)

            # encoder
            self.fib_enc = dab.fic_encode(self.dp)
            self.msc_encoder = dab.msc_encode(self.dp, 14, 2)
            self.msc_encoder2 = dab.msc_encode(self.dp, 14, 2)

            # multiplexer
            self.mux = dab.dab_transmission_frame_mux_bb_make(1, 2, [84, 84])

            # mapper
            self.unpack = blocks.packed_to_unpacked_bb_make(1, gr.GR_MSB_FIRST)
            self.map = dab.mapper_bc_make(self.dp.num_carriers)

            # demapper
            self.s2v = blocks.stream_to_vector_make(gr.sizeof_gr_complex, self.dp.num_carriers)
            self.soft_interleaver = dab.complex_to_interleaved_float_vcf_make(self.dp.num_carriers)

            # decode
            self.fic_decoder = dab.fic_decode(self.dp)
            self.msc_dec = dab.dabplus_audio_decoder_ff(self.dp, 112, 0, 84, 2, True)

            # audio sink
            self.audio = audio.sink_make(32000)

            # control stream
            self.trigger_src = blocks.vector_source_b([1] + [0] * 74, True)

            # connect everything
            self.tb.connect(self.fib_src, self.fib_enc, (self.mux, 0))
            self.tb.connect(self.subch_src01, self.msc_encoder, (self.mux, 1))
            self.tb.connect(self.subch_src02, self.msc_encoder2, (self.mux, 2))
            self.tb.connect((self.mux, 0), self.unpack, self.map, self.s2v, self.soft_interleaver, (self.msc_dec, 0))
            self.tb.connect(self.soft_interleaver, (self.fic_decoder, 0))
            self.tb.connect(self.trigger_src, (self.fic_decoder, 1))
            self.tb.connect(self.trigger_src, (self.msc_dec, 1))
            self.tb.connect((self.msc_dec, 0), (self.audio, 0))
            self.tb.connect((self.msc_dec, 1), (self.audio, 1))

            self.tb.run ()
            pass
        else:
            log.debug("debug file not found - skipped test")
            log.set_level("WARN")
            pass


if __name__ == '__main__':
    gr_unittest.run(qa_msc_encode, "qa_msc_encode.xml")
