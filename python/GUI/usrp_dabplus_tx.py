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

"""
transmit DAB+ with USRP
"""

from gnuradio import gr, uhd, blocks
from gnuradio import audio
import dab
import numpy as np


class usrp_dabplus_tx(gr.top_block):
    def __init__(self, dab_mode, frequency, num_subch, ensemble_label, service_label, language, protections, data_rates_n, src_paths, selected_audio, use_usrp, dabplus_types, sink_path = "dab_iq_generated.dat"):
        gr.top_block.__init__(self)

        self.dab_mode = dab_mode
        self.frequency = frequency
        interp = 64
        self.sample_rate = 128e6 / interp
        self.dp = dab.parameters.dab_parameters(self.dab_mode, 2000000, False)
        self.num_subch = num_subch
        self.ensemble_label = ensemble_label
        self.service_label = service_label
        self.language = language
        self.protections = protections
        self.data_rates_n = data_rates_n
        self.subch_sizes = np.multiply(self.data_rates_n, 6)
        self.src_paths = src_paths
        self.use_usrp = use_usrp
        self.dabplus_types = dabplus_types
        self.sink_path = sink_path
        self.selected_audio = selected_audio
        self.volume = 80

        ########################
        # FIC
        ########################
        # source
        self.fic_src = dab.fib_source_b_make(self.dab_mode, self.num_subch, self.ensemble_label, self.service_label, "", self.language, self.protections, self.data_rates_n, self.dabplus_types)
        # encoder
        self.fic_enc = dab.fic_encode(self.dp)

        ########################
        # MSC
        ########################
        self.recorder = audio.source_make(32000)
        self.msc_sources = [None] * self.num_subch
        self.f2s_left_converters = [None] * self.num_subch
        self.f2s_right_converters = [None] * self.num_subch
        self.mp4_encoders = [None] * self.num_subch
        self.mp2_encoders = [None] * self.num_subch
        self.rs_encoders = [None] * self.num_subch
        self.msc_encoders = [None] * self.num_subch
        for i in range(0, self.num_subch):
            if not self.src_paths[i] is "mic":
                # source
                self.msc_sources[i] = blocks.wavfile_source_make(self.src_paths[i], True)
            # float to short
            self.f2s_left_converters[i] = blocks.float_to_short_make(1, 32767)
            self.f2s_right_converters[i] = blocks.float_to_short_make(1, 32767)
            if self.dabplus_types[i] is 1:
                # mp4 encoder and Reed-Solomon encoder
                self.mp4_encoders[i] = dab.mp4_encode_sb_make(self.data_rates_n[i], 2, 32000, 1)
                self.rs_encoders[i] = dab.reed_solomon_encode_bb_make(self.data_rates_n[i])
            else:
                # mp2 encoder
                self.mp2_encoders[i] = dab.mp2_encode_sb_make(self.data_rates_n[i], 2, 48000)
            # encoder
            self.msc_encoders[i] = dab.msc_encode(self.dp, self.data_rates_n[i], self.protections[i])

        ########################
        # MUX
        ########################
        self.mux = dab.dab_transmission_frame_mux_bb_make(self.dab_mode, self.num_subch, self.subch_sizes)
        #self.mux = dab.dab_transmission_frame_mux_bb_make(1, 1, [84, 84])
        self.trigsrc = blocks.vector_source_b([1] + [0] * (self.dp.symbols_per_frame-2), True)

        ########################
        # Modulator
        ########################
        self.s2v_mod = blocks.stream_to_vector(gr.sizeof_char, self.dp.num_carriers/4)
        self.mod = dab.ofdm_mod(self.dp)

        ########################
        # Sink
        ########################
        if self.use_usrp:
            self.sink = uhd.usrp_sink("", uhd.io_type.COMPLEX_FLOAT32, 1)
            self.sink.set_samp_rate(self.sample_rate)
            self.sink.set_antenna("TX/RX")
            self.sink.set_center_freq(self.frequency)
        else:
            self.sink = blocks.file_sink_make(gr.sizeof_gr_complex, self.sink_path)
        # audio sink
        self.audio = audio.sink_make(32000)

        self.gain_left = blocks.multiply_const_ff_make(1, 1)
        self.gain_right = blocks.multiply_const_ff_make(1, 1)
        self.s2f_left = blocks.short_to_float_make(1, 32767)
        self.s2f_right = blocks.short_to_float_make(1, 32767)

        ########################
        # Connections
        ########################
        self.connect(self.fic_src, self.fic_enc, (self.mux, 0))
        for i in range(0, self.num_subch):
            if self.dabplus_types[i] is 1:
                if self.src_paths[i] is "mic":
                    self.connect((self.recorder, 0), self.f2s_left_converters[i], (self.mp4_encoders[i], 0), self.rs_encoders[i], self.msc_encoders[i], (self.mux, i + 1))
                    self.connect((self.recorder, 1), self.f2s_right_converters[i], (self.mp4_encoders[i], 1))
                else:
                    self.connect((self.msc_sources[i], 0), self.f2s_left_converters[i], (self.mp4_encoders[i], 0), self.rs_encoders[i], self.msc_encoders[i], (self.mux, i+1))
                    self.connect((self.msc_sources[i], 1), self.f2s_right_converters[i], (self.mp4_encoders[i], 1))
            else:
                self.connect((self.msc_sources[i], 0), self.f2s_left_converters[i], (self.mp2_encoders[i], 0), self.msc_encoders[i], (self.mux, i + 1))
                self.connect((self.msc_sources[i], 1), self.f2s_right_converters[i], (self.mp2_encoders[i], 1))
        self.connect((self.mux, 0), self.s2v_mod, (self.mod, 0))
        self.connect(self.trigsrc, (self.mod, 1))
        self.connect(self.mod, blocks.throttle_make(gr.sizeof_gr_complex, 2e6), self.sink)
        self.connect((self.msc_sources[self.selected_audio-1], 0), self.gain_left, (self.audio, 0))
        self.connect((self.msc_sources[self.selected_audio-1], 1), self.gain_right, (self.audio, 1))

    def transmit(self):
        tx = usrp_dabplus_tx()
        tx.run()

    def set_volume(self, volume):
        self.gain_left.set_k(volume)
        self.gain_right.set_k(volume)

