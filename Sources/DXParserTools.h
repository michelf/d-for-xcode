
// D for Xcode: Parser Tools
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
