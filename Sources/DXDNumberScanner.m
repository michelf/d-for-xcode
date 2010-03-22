
// D for Xcode: Number Scanner for Xcode 3
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

#import "DXDNumberScanner.h"
#import "DXScannerTools.h"
#import "XCCharStream.h"

enum Response { notoken = -1, token = 65542 };

@implementation DXDNumberScanner

- (int)tokenForString:(NSString *)str forRange:(NSRange *)range subItems:(id *)subItems {
	size_t length = numberLength(str);
	if (length) {
		range->location = 0;
		range->length = length;
		return token;
	}
	return [super tokenForString:str forRange:range subItems:subItems];
}

@end
