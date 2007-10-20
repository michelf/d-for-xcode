//
//  DXParserTools.h
//  D for Xcode
//
//  Created by Michel Fortin on 2007-09-29.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

// Skip system imports when including from a file which must not include 
// system libraries.
#ifndef DX_PURE_DMD
#import <Foundation/Foundation.h>
#import "XCPSourceParsing.h"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * Initialize the D parser. This must be called before doing anything else
 * with the D parser, but may be called multiple times.
 */
void DXParserInit();

// Skip these when including from a file which must not include system 
// libraries.
#ifndef DX_PURE_DMD

/*!
 * Determine file dependencies for input source.
 */
NSSet *DXSourceDependenciesForPath(NSString *input);

/*!
 * Scan code for symbols and return scanner items.
 */
PBXSourceScannerItem *DXScanSource(NSString *code);
#endif

#if defined(__cplusplus)
}
#endif
