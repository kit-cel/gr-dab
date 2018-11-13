gr-dab
======

A Digital Audio Broadcasting and Digital Audio Broadcasting + module for GNU Radio 

Contents
--------

0: License

1: Google Summer of Code 2017

2: Installation

3: Usage

4: Features

5: Current Constraints

6: Ideas

7: Known Bugs

8: Documentation

License
-------
Copyright (C) Andreas Müller, 2011, Moritz Luca Schmid, 2017

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Google Summer of Code 2017
------------

This repository is the result of the 2017 Google Summer of Code project "A DAB/DAB+ transceiver app" by Moritz Luca Schmid. It is a fork of the [gr-dab](https://github.com/andrmuel/gr-dab) repository of Andreas Mueller which already contains the implementation of the DAB physical layer including OFDM. The following expanding work on gr-dab was done during GSoC 2017 by Moritz Luca Schmid:

* FIC source and expand FIC sink (GNU Radio blocks)
* FIC encoder, including CRC, energy dispersal, convolutional encoding with puncturing (GNU Radio blocks)
* MSC encoder and decoder, including CRC, energy dispersal/descramble, convolutional encoding/decoding with puncturing, time interleaving/deinterleaving (GNU Radio blocks)
* DAB transmission frame multiplexer (GNU Radio block)
* MPEG Audio Layer II encoder/decoder for DAB tranmitter/receiver (GNU Radio blocks)
* HE-AAC v2 (mp4) encoder/decoder for DAB+ transmitter/receiver (GNU Radio blocks)
* Graphical user interface for DAB/DAB+ transmission/reception (pyQt4)
* "DABstep" as an executable application for DAB/DAB+ transmission/reception

For more detailed information about the work done during GSoC, containing changes in code and new code, check out the [commit history](https://github.com/kit-cel/gr-dab/commits/master) of this repository. Weekly reports containing updates about the working progress, additional information, highlights and challenging pieces during GSoC can be found on my [GSoC blog](https://dabtransceiver.wordpress.com/).

Summarizing, all set
[milestones](https://dabtransceiver.wordpress.com/milestones/) for GSoC were
fulfilled. All my code got merged into the master branch of this repository.
This fork will serve as upstream repository for further development of the OFDM
receiver (especially with respect to synchronization), and other improvements.

Installation
------------

This directory (and the resulting tarball) contains a build tree for
gr-dab.

This package requires that GNU Radio is already installed.  It
also depends on some GNU Radio prerequisites, such as Boost and
cppunit.

A dependency is the FAAD2 library. (ubuntu: sudo apt-get install libfaad-dev, fedora: sudo dnf install faad2-devel)

It also depends on fdk-aac with DAB patches (fdk-aac-dab). You'll find it in
the eponymous subdirectory; build it using:

    $ cd fdk-aac-dab
    $ ./bootstrap
    $ ./configure [--prefix ...] [--other options]
    $ make
    $ make install

There is a dependency on the modified MPEG encoder libtoolame from the ODR
project. It's fetched by ´git submodule update --init´ automatically.

To build this module, run these commands:

    $ git submodule update --init
    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ sudo make install
    $ sudo ldconfig
 


Features
--------

* Receiving and Transmitting DAB/DAB+ broadcasts with the graphical application "DABstep"
* Receiving or Transmitting DAB/DAB+ with prepaired GNU Radio Companion flowgraph in examples/
* Receiving or Transmitting DAB/DAB+ by building your own GNU Radio flowgraph with provided gr-dab blocks
* USRP and RTL-SDR for reception supported

Usage
-------

* GUI
  * execute "DABstep" over command line
  * run python/GUI/main.py

* Flowgraphs
  * run the flowgraphs in examples/ with GNU Radio Companion

Current Constraints
---------------------

* only audio channels supported so far, no data channels
* PAD interpretation and generation not supported so far
* only EEP with protection profile set A supported so far, no set B and no UEP


Ideas
----------

* FM simulation, calculating the audio noise out of the SNR and adding it to the audio
* parallel FM receiver that fills in the audio in case a superframe is broken 
* include reception and transmission of data channels with packed data, e.g. pictures

Known Bugs
----------

* Audio underruns disturb played audio signal without having actual errors. Additional buffer through delay block is a temporary fix that only works until reception errors occur.

Documentation
--------------

You can build the Doxygen documentation of the c++ classes in lib/ with this command:
    $ doxygen doxygen-config

