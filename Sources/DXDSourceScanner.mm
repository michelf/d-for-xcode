//
//  DXDSourceScanner.m
//  D for Xcode
//
//  Created by Michel Fortin on 2007-09-29.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "DXDSourceScanner.h"
#import "DXParserTools.h"

#undef check
#include "parse.h"
#include "import.h"
#include "dsymbol.h"
#include "identifier.h"


@implementation DXDSourceScanner

- (PBXSourceScannerItem *)parseString:(NSString *)code {
	// Implementation in ParserTools.
	return DXScanSource(code);
}

@end
