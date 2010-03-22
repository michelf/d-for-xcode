
// D for Xcode: Test for Xcode 3 Number Scanner
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

#import "DXDNumberScannerTest.h"
#import "DXScannerTools.h"

@implementation DXDNumberScannerTest

- (void)testVariousNumbers {
	NSString *input;
	NSEnumerator *goodEnumerator = [[NSArray arrayWithObjects:
		@"123",
		@"0",
		@".12",
		@"1_2_3",
		@"1_2._3",
		@"18_446_744_073_709_551_615UL",
		@"0xabcd_ef12_ABCD_EF34UL",
		@"123_456.567_8",
		@"1_2_3_4_5_6_._5e-6_",
		@"0x1.FFFFFFFFFFFFFp1023",
		@"0x1p-52",
		@"1.175494351e-38F",
		@"6.3Li",
		@"6.2i",
		@"1234_12349",
		nil] objectEnumerator];
	NSEnumerator *badEnumerator = [[NSArray arrayWithObjects:
		@".h",
		@"a",
		@"12a",
		@"._12",
		@"01234_12349",
		nil] objectEnumerator];
	
	while (input = [goodEnumerator nextObject]) {
		STAssertTrue(isNumber(input),
			@"Could not parse \"%@\" as a number.", input);
	}
	while (input = [badEnumerator nextObject]) {
		STAssertFalse(isNumber(input),
			@"Could parse \"%@\" as a number.", input);
	}
}

@end
