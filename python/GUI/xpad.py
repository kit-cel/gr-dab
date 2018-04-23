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

from PyQt4.QtCore import QObject, pyqtSignal

class xpad(QObject):
    """
    Sub-class of QObject, used to communicate from the gr block xpad_message_handler
    to the GUI, using pyQt signals.
    """
    # define a new pyQt signal for the signalling and transport of a new dynamic_label/mot_image
    new_xpad = pyqtSignal('QString', name='new_xpad')

    def __init__(self, slot, parent=None):
        super(xpad, self).__init__(parent)
        # connect the signal new_label/new_image to the slot callable
        self.new_xpad.connect(slot)

    def emit_label(self, data):
        # emit signal now
        self.new_xpad.emit(data)
