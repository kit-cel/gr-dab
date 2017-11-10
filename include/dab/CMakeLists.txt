# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Install public header files
########################################################################
install(FILES
    api.h
    diff_phasor_vcc.h
    frequency_interleaver_vcc.h
    qpsk_demapper_vcb.h
    complex_to_interleaved_float_vcf.h
    select_vectors.h
    repartition_vectors.h
    unpuncture_vff.h
    prune_vectors.h
    fib_sink_vb.h
    qpsk_mapper_vbc.h
    ofdm_insert_pilot_vcc.h
    sum_phasor_trig_vcc.h
    ofdm_move_and_insert_zero.h
    insert_null_symbol.h
    time_interleave_bb.h
    time_deinterleave_ff.h
    crc16_bb.h
    fib_source_b.h
    select_subch_vfvf.h
    unpuncture_ff.h
    prune.h
    firecode_check_bb.h
    puncture_bb.h
    dab_transmission_frame_mux_bb.h
    conv_encoder_bb.h
    mapper_bc.h
    mp2_decode_bs.h
    mp4_decode_bs.h
    reed_solomon_decode_bb.h
    reed_solomon_encode_bb.h
    mp4_encode_sb.h
    mp2_encode_sb.h
    valve_ff.h
    synchronization_ff.h
    ofdm_synchronization_cvf.h
    ofdm_coarse_frequency_correction_vcvc.h
    phase_correction_cc.h
    frequency_deinterleave_cc.h
    demux_cc.h
    select_cus_vcvc.h
    qpsk_mapper_vbvc.h DESTINATION include/dab
)
