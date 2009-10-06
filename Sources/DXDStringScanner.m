
// D for Xcode: String Scanner for Xcode 3
// Copyright (C) 2008  Michel Fortin
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

#import "DXDStringScanner.h"
#import "DXScannerTools.h"
#import "XCSourceModelItem.h"
#import "XCCharStream.h"
#import "XCPSupport.h"

enum Response { notoken = -1, token = 65542 };

@implementation DXDStringScanner

- (id)parse:(id)a withContext:(id)b initialToken:(int)c inputStream:(XCCharStream *)d range:(NSRange)e dirtyRange:(NSRange *)f {
	
	NSString *str = [d stringWithRange:e];
	str = [str stringByDeletingLeadingWhitespace];
	e.location += e.length - [str length];
	
	// Skip if not a ddoc comment.
	if ([self tokenStringOnly]) {
		if ([str hasPrefix:@"q{"])
			return nil;
	}
	
	size_t foundStringLength = stringLength(str);
	if (foundStringLength) {
		e.length = foundStringLength;

//		NSLog(@"string -> location = %d -> %d", [d location], e.location+e.length);
		[d setLocation:e.location+e.length];

//		NSLog(@"string -> %@", [str substringWithRange:NSMakeRange(0, e.length)]);
		XCSourceModelItem *r = [super parse:nil withContext:b initialToken:c inputStream:d range:e dirtyRange:f];
		[r setNeedToDirtyRightEdges:YES];
		return r;

	}
	return nil;
}

- (BOOL)predictsRule:(int)tokenType inputStream:(XCCharStream *)stream {
	size_t location = [stream location]-1;
	unichar c = location < [stream length] ? [[stream string] characterAtIndex:location] : 0;
	BOOL result = c == '"' || c == '`' || c == 'r' || c == 'q' || c == 'x';
//	NSLog(@"string -> predict rule (%d) = %d", tokenType, result);
	return result;
}

- (BOOL)tokenStringOnly {
	return NO;
}

- (BOOL)canTokenize {
	return YES;
}

@end

@implementation DXDTokenStringScanner

- (BOOL)tokenStringOnly {
	return YES;
}

@end