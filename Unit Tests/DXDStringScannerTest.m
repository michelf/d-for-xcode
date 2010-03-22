
// Unit tests for string scanner
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

#import "DXDStringScannerTest.h"
#import "DXScannerTools.h"


@implementation DXDStringScannerTest

- (void)testVariousStrings {
	NSString *input;
	NSEnumerator *partialEnumerator = [[NSArray arrayWithObjects:
		@"\"...\\\"...\" ",
		@"r\"...\\\" ",
		@"`...\\` ",
		@"x\"...\" ",
		@"q\"@...@\" ",
		@"q\"<...>\" ",
		@"q\"ABC\n...\nABC\" ",
		@"q\"ABC\n...\nABC\"  ",
		@"q{...{...}...} ",
		// with invalid suffix
		@"\"...\\\"...\"n",
		@"r\"...\\\"o",
		@"`...\\`*",
		@"x\"...\"!",
		@"q\"@...@\"r",
		@"q\"<...>\"u",
		@"q\"ABC\n...\nABC\"v",
		@"q{...{...}...}L",
		nil] objectEnumerator];
	NSEnumerator *fullEnumerator = [[NSArray arrayWithObjects:
		@"\"...\\\"...",
		@"r\"...",
		@"`...",
		@"x\"...",
		@"q\"@...",
		@"q\"<...",
		@"q\"ABC\n...",
		@"q{...{...}",
		// with valid suffix
		@"\"...\\\"...\"c",
		@"r\"...\\\"w",
		@"`...\\`d",
		@"x\"...\"c",
		@"q\"@...@\"w",
		@"q\"<...>\"d",
		@"q\"ABC\n...\nABC\"c",
		@"q{...{...}...}w",
		nil] objectEnumerator];
	
	while (input = [partialEnumerator nextObject]) {
		STAssertTrue(stringLength(input) != 0 && stringLength(input) < [input length],
			@"Parsing of \"%@\" failed (partial match). [%d]", input, stringLength(input));
	}
	while (input = [fullEnumerator nextObject]) {
		STAssertTrue(stringLength(input) == [input length],
			@"Parsing of \"%@\" failed (full match). [%d]", input, stringLength(input));
	}
}

@end
