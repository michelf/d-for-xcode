
// D for Xcode: Source Scanner Item
// Copyright (C) 2007  Michel Fortin
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

#import <Cocoa/Cocoa.h>
#import "XCPSourceParsing.h"

struct Dsymbol;
struct Array;

void DXPopulateItemWithSymbols(PBXSourceScannerItem *item, Array *symbols);

// Scanner Item Types
enum DXType {
	DXTypeRoot,
	DXTypeMark,
	DXTypeClassDecl, DXTypeClass,
	DXTypeMethodDecl, DXTypeMethod,
	DXTypeFunctionDecl, DXTypeFunction,
	DXTypePreprocessorDefine = 8,
	DXTypeTypedef = 11,
};

@interface DXDSourceScannerItem : PBXSourceScannerItem {
	Dsymbol *symbol;
}

- (id)initWithSymbol:(Dsymbol *)s;

@end
