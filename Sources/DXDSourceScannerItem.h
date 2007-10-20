//
//  DXDSourceScannerItems.h
//  D for Xcode
//
//  Created by Michel Fortin on 2007-10-02.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

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
