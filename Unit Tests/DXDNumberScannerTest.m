//
//  DXDNumberScannerTest.m
//  D for Xcode
//
//  Created by Michel Fortin on 2008-04-08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

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
