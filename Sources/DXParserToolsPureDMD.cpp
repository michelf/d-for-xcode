
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