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
from PyQt4.QtGui import QImage
from PIL import Image
import numpy as np

class xpad(QObject):
    """
    Sub-class of QObject, used to communicate from the gr block xpad_message_handler
    to the GUI, using pyQt signals.
    """
    # define a new pyQt signal for the signalling and transport of a new dynamic_label/mot_image
    new_label = pyqtSignal('QString', name='new_xpad_label')
    new_image = pyqtSignal(np.ndarray, name='new_xpad_image')

    def __init__(self, slot, type, parent=None):
        super(xpad, self).__init__(parent)
        self.type = type
        # connect the signal new_label/new_image to the slot callable
        if (type == 'xpad_label'):
            self.new_label.connect(slot)
        elif (type == 'xpad_image'):
            self.new_image.connect(slot)

    def emit_label(self, msg_data):
        # emit signal now
        if (self.type == 'xpad_label'):
            print "emit xpad py qt signal: label"
            self.new_label.emit(msg_data)
        elif (self.type == 'xpad_image'):
            print "emit xpad py qt signal: image"
            print msg_data
            #qt_image = QImage(interpreted_image.data, 320, 240, QImage.Format_Indexed8)
            self.new_image.emit(msg_data)
