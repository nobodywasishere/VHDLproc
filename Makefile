#
#  VHDLproc - VHDL preprocessor
#  Copyright (c) 2020 Michael Riegert <michael@eowyn.net>
#

#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 3
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

CC=gcc
INSTALL_PREFIX=/usr/local/bin
PROGNAME=vhdlproc
PROGVER='"1.0"'
SRCDIR=../src
TESTS=(comment define for include message rand undef)

all:
	-mkdir -p build
	-cd build && rm vhdlproc
	-cd build && $(CC) -DVERSION=$(PROGVER) -o $(PROGNAME) $(SRCDIR)/*.h $(SRCDIR)/*.c
	
test:
	for t in ${TESTS[@]}; do echo $t; done

install:
	sudo mkdir -p $(INSTALL_PREFIX)
	sudo cp build/$(PROGNAME) $(INSTALL_PREFIX)

clean:
	rm -f tests/*-out.vhdl
	rm -f build/vhdlproc
