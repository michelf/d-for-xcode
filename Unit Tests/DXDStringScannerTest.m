//
//  DXDStringScannerTest.m
//  D for Xcode
//
//  Created by Michel Fortin on 2009-10-04.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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
