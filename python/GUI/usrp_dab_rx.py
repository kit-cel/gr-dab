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
receive DAB+ with USRP
"""

from gnuradio import gr, uhd, blocks
from gnuradio import audio, digital
from gnuradio import qtgui
from gnuradio import fft
import osmosdr
import dab
import time, math


class usrp_dab_rx(gr.top_block):
    def __init__(self, dab_mode, frequency, bit_rate, address, size, protection, audio_bit_rate, dabplus, use_usrp, use_rtl, src_path, sink_path = "None", prev_src=None):
        gr.top_block.__init__(self)

        self.dab_mode = dab_mode
        self.verbose = False
        self.sample_rate = 2048e3
        self.dabplus = dabplus
        self.use_usrp = use_usrp
        self.use_rtl = use_rtl
        self.src_path = src_path
        self.sink_path = sink_path
        gr.log.set_level("warn")

        # set paramters to default mode
        self.softbits = True
        self.filter_input = True
        self.autocorrect_sample_rate = False
        self.resample_fixed = 1
        self.correct_ffe = True
        self.equalize_magnitude = True
        self.frequency = frequency
        self.dab_params = dab.parameters.dab_parameters(self.dab_mode, self.sample_rate, self.verbose)
        self.rx_params = dab.parameters.receiver_parameters(self.dab_mode, self.softbits,
                                                            self.filter_input,
                                                            self.autocorrect_sample_rate,
                                                            self.resample_fixed,
                                                            self.verbose, self.correct_ffe,
                                                            self.equalize_magnitude)

        ########################
        # source
        ########################
        if self.use_usrp:
            self.src = uhd.usrp_source("", uhd.io_type.COMPLEX_FLOAT32, 1)
            self.src.set_clock_rate(self.sample_rate*8)
            self.src.set_samp_rate(self.sample_rate)
            self.src.set_gain(50, 0)
            self.src.set_antenna("TX/RX")

        elif self.use_rtl:
            if(prev_src is None):
                self.src = osmosdr.source()
            else:
                self.src = prev_src
            self.src.set_sample_rate(self.sample_rate)
            self.src.set_center_freq(self.frequency)
            self.src.set_freq_corr(0, 0)
            self.src.set_dc_offset_mode(2, 0)
            self.src.set_iq_balance_mode(0, 0)
            self.src.set_gain_mode(False, 0)
            self.src.set_gain(50, 0)
            self.src.set_if_gain(20, 0)
            self.src.set_bb_gain(20, 0)
            self.src.set_antenna("", 0)
            self.src.set_bandwidth(0, 0)
        else:
            print "using file source"
            self.src = blocks.file_source_make(gr.sizeof_gr_complex, self.src_path, False)

        ########################
        # FFT and waterfall plot
        ########################
        self.fft_plot = qtgui.freq_sink_c_make(1024, fft.window.WIN_BLACKMAN_HARRIS, self.frequency, 2e6, "FFT")
        self.waterfall_plot = qtgui.waterfall_sink_c_make(1024, fft.window.WIN_BLACKMAN_HARRIS, self.frequency, 2e6, "Waterfall")
        #self.time_plot = qtgui.time_sink_c_make(1024, 2e6, "Time")

        ########################
        # OFDM demod
        ########################
        self.demod = dab.ofdm_demod_cc(self.dab_params)

        ########################
        # SNR measurement
        ########################
        self.v2s_snr = blocks.vector_to_stream(gr.sizeof_gr_complex*1, self.dab_params.num_carriers)
        self.constellation_plot = qtgui.const_sink_c_make(1024, "", 1)
        self.constellation_plot.enable_grid(True)

        ########################
        # FIC decoder
        ########################
        self.fic_dec = dab.fic_decode_vc(self.dab_params)

        ########################
        # MSC decoder
        ########################
        if self.dabplus:
            self.dabplus = dab.dabplus_audio_decoder_ff(self.dab_params, bit_rate, address, size, protection, True)
            # xpad handler
            self.pad_messenger = dab.xpad_message_handler()
        else:
            self.msc_dec = dab.msc_decode(self.dab_params, address, size, protection)
            self.unpack = blocks.packed_to_unpacked_bb_make(1, gr.GR_MSB_FIRST)
            self.mp2_dec = dab.mp2_decode_bs_make(bit_rate / 8)
            self.s2f_left = blocks.short_to_float_make(1, 32767)
            self.s2f_right = blocks.short_to_float_make(1, 32767)
            self.gain_left = blocks.multiply_const_ff(1, 1)
            self.gain_right = blocks.multiply_const_ff(1, 1)

        ########################
        # audio sink
        ########################
        self.valve_left = dab.valve_ff_make(True)
        self.valve_right= dab.valve_ff_make(True)
        self.delay_left = blocks.delay_make(gr.sizeof_float, int(audio_bit_rate))
        self.delay_right = blocks.delay_make(gr.sizeof_float, int(audio_bit_rate))
        self.audio = audio.sink_make(audio_bit_rate)
        self.wav_sink = blocks.wavfile_sink_make("dab_audio.wav", 2, audio_bit_rate)

        ########################
        # Connections
        ########################
        self.connect(self.src, self.fft_plot)
        self.connect(self.src, self.waterfall_plot)
        self.connect(self.src, self.demod, self.fic_dec)
        if self.dabplus:
            self.connect((self.demod, 1), self.dabplus)
        else:
            self.connect((self.demod, 0), (self.msc_dec, 0), self.unpack, self.mp2_dec)
            self.connect((self.demod, 1), (self.msc_dec, 1))
            self.connect((self.mp2_dec, 0), self.s2f_left, self.gain_left)
            self.connect((self.mp2_dec, 1), self.s2f_right, self.gain_right)
        self.connect((self.demod, 0), self.v2s_snr, self.constellation_plot)
        # connect audio to sound card and file sink
        if self.dabplus:
            self.connect((self.dabplus, 0), self.delay_left, (self.audio, 0))
            self.connect((self.dabplus, 1), self.delay_right, (self.audio, 1))
            self.connect((self.dabplus, 0), self.valve_left, (self.wav_sink, 0))
            self.connect((self.dabplus, 1), self.valve_right, (self.wav_sink, 1))
            # connect msg port for dynamic labels
            self.msg_connect(self.dabplus, "dynamic_label", self.pad_messenger, "dynamic_label")
        else:
            self.connect(self.gain_left, self.delay_left, (self.audio, 0))
            self.connect(self.gain_right, self.delay_right, (self.audio, 1))
            self.connect(self.gain_left, self.valve_left, (self.wav_sink, 0))
            self.connect(self.gain_right, self.valve_right, (self.wav_sink, 1))

        # tune USRP frequency
        if self.use_usrp:
            self.set_freq(self.frequency)


########################
# getter methods
########################
    def get_ensemble_info(self):
        return self.fic_dec.get_ensemble_info()

    def get_service_info(self):
        return self.fic_dec.get_service_info()

    def get_service_labels(self):
        return self.fic_dec.get_service_labels()

    def get_subch_info(self):
        return self.fic_dec.get_subch_info()

    def get_programme_type(self):
        return self.fic_dec.get_programme_type()

    def get_sample_rate(self):
        return self.dabplus.get_sample_rate()

    def get_snr(self):
        return self.demod.get_snr()

    def get_firecode_passed(self):
        return self.dabplus.get_firecode_passed()

    def get_corrected_errors(self):
        return self.dabplus.get_corrected_errors()

    def get_crc_passed(self):
        return self.fic_dec.get_crc_passed()

########################
# setter methods
########################
    def set_volume(self, volume):
        if self.dabplus:
            self.dabplus.set_volume(volume)
        else:
            self.gain_left.set_k(volume)
            self.gain_right.set_k(volume)

    def set_valve_closed(self, closed):
        self.valve_left.set_closed(closed)
        self.valve_right.set_closed(closed)

    def set_freq(self, freq):
        if self.src.set_center_freq(freq):
            if self.verbose:
                print "--> retuned to " + str(freq) + " Hz"
            return True
        else:
            print "-> error - cannot tune to " + str(freq) + " Hz"
            return False

    def set_gain(self, gain):
        if hasattr(self, 'src'):
            self.src.set_gain(gain, 0)

    def receive(self):
        rx = usrp_dab_rx()
        rx.run()

