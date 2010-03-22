
// D for Xcode: Test for Xcode 3 Comment Scanner
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

#import "DXDCommentScannerTest.h"
#import "DXScannerTools.h"


@implementation DXDCommentScannerTest

- (void)testVariousNumbers {
	NSString *input;
	NSEnumerator *partialEnumerator = [[NSArray arrayWithObjects:
		@"// \n",
		@"/* \n */ ",
		@"/+ /+ \n +/ +/ ",
		nil] objectEnumerator];
	NSEnumerator *fullEnumerator = [[NSArray arrayWithObjects:
		@"// ",
		@"/* \n",
		@"/+ /+ \n +/ ",
		nil] objectEnumerator];
	
	while (input = [partialEnumerator nextObject]) {
		STAssertTrue(commentLength(input) != 0 && commentLength(input) < [input length],
			@"Parsing of \"%@\" failed (partial match).", input);
	}
	while (input = [fullEnumerator nextObject]) {
		STAssertTrue(commentLength(input) == [input length],
			@"Parsing of \"%@\" failed (full match).", input);
	}
}

@end
