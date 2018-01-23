# Copyright 2011 Free Software Foundation, Inc.
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

include(GrPython)

GR_PYTHON_INSTALL(
    FILES
    usrp_dab_rx.py
    usrp_dab_tx.py
    user_frontend.py
    DESTINATION ${GR_PYTHON_DIR}/dab
    COMPONENT "dab_python"
)
set(GR_PKG_DAB_DATA_DIR ${GR_PKG_DATA_DIR})
install(FILES                                                                   
   DAB_logo.png
   led_red.png
   led_orange.png
   led_green.png                                                             
   DESTINATION ${GR_PKG_DAB_DATA_DIR}                                          
)

########################################################################
# Generate the GUI python code user_frontend.py from the user interface  user_frontend.ui
########################################################################
set(GUI_UI ${CMAKE_CURRENT_SOURCE_DIR}/user_frontend.ui)
set(GUI_PY ${CMAKE_CURRENT_BINARY_DIR}/user_frontend.py)
add_custom_command(OUTPUT ${GUI_UI}
    COMMAND ${PYUIC4_EXECUTABLE} pyuic4 ${GUI_UI} -o ${GUI_PY}
    DEPENDS ${GUI_UI}
    COMMENT "Generating python code from the user interface"
)
