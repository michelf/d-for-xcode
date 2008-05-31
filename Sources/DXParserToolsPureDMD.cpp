
// D for Xcode: Parser Tools
// Copyright (C) 2007-2008  Michel Fortin
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

// These includes interfere with symbols imported from other system headers so
// we need to used them together in a separate file. DXParserInit is declared 
// in DXParserTools.h.

#define DX_PURE_DMD

#include "DXParserTools.h"

#include "mtype.h"
#include "id.h"
#include "module.h"
#include "lexer.h"

void DXParserInit() {
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		
		Type::init();
		Id::initialize();
		Module::init();
		
		global.gag = 1; // gag reporting of errors.
	}
}