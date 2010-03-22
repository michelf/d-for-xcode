
// D for Xcode: DMD redirection
// Copyright (c) 2007-2010  Michel Fortin
//
// D for Xcode is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
//
// D for Xcode is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
// more details.
//
// You should have received a copy of the GNU General Public License
// along with D for Xcode; if not, write to the Free Software Foundation, 
// Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

// This program just launches DMD found in another directory, because a 
// symbolic link is not enough.

#include <unistd.h>
#include <stdio.h>

#ifndef DX_DMD_PATH
#error Must set DX_DMD_PATH
#endif

int main(unsigned int argc, char **argv) {
	char *command = argv[0];
	argv[0] = DX_DMD_PATH;
	execv(DX_DMD_PATH, argv);
	fprintf(stderr, "%s: failed to launch executable at %s.", command, DX_DMD_PATH);
	return -1;
}