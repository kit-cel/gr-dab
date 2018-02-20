#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL)
# Karlsruhe Institute of Technology (KIT).
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

from gnuradio import gr
import pmt

class xpad_message_handler(gr.basic_block):
    """
    @brief block to receive xpad messages from mp4 decoder and create pyQt signals

    currently supported xpad messages:
    -dynamic labels
    """
    def __init__(self, qt_dyn_lab):
        gr.basic_block.__init__(self,
            name="xpad_message_handler",
            in_sig=None,
            out_sig=None)

        # define input message port for dynamic labels
        self.message_port_register_in(pmt.intern("dynamic_label"))
        self.set_msg_handler(pmt.intern("dynamic_label"), self.handle_dynamic_label)
        self.qt_dyn_lab = qt_dyn_lab

    def handle_dynamic_label(self, msg):
        # convert numpy utf8 byte array to python string
        label = pmt.to_python(msg)
        unicode_object = unicode(label.data, "utf8")
        # emit a signal to the GUI, containing this dynamic label string
        self.qt_dyn_lab.emit_label(unicode_object)
