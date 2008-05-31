
// D for Xcode: Source Scanner
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

#import <XCPSourceParsing.h>
#import <XCPSpecifications.h>
#import <DXDSourceLexer.h>


/*!
 * Source code scanner class used by Xcode to create the editor's symbol menu.
 */
@interface DXDSourceScanner : PBXSourceScanner {
}

/*!
 * Parse a string and return the root PBXSourceScannerItem for the document.
 */
- (PBXSourceScannerItem *)parseString:(NSString *)code;

@end
