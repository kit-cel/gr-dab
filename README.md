gr-dab
======

A Digital Audio Broadcasting and Digital Audio Broadcasting + module for GNU Radio 

Contents
--------

0: License

1: Installation

2: Usage

3: Features

4: (Current) Constraints

5: Known Bugs

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


Installation
------------

This directory (and the resulting tarball) contains a build tree for
gr-dab.

This package requires that GNU Radio is already installed.  It
also depends on some GNU Radio prerequisites, such as Boost and
cppunit.

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
 


Usage
-----

Receiving DAB+ broadcasts
#########################


Transmitting DAB+
#################


Features
--------

* Transmits DAB+ Audio transmissions, metadata
* Receives DAB+

(Current) Constraints
---------------------

TODO

Known Bugs
----------

