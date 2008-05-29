
// D for Xcode: Number Scanner for Xcode 3
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

#import "DXDNumberScanner.h"
#import "DXScannerTools.h"

enum Response { notoken = -1, token = 65542 };

@implementation DXDNumberScanner

- (int)tokenForString:(NSString *)str forRange:(NSRange *)range subItems:(id *)subItems {
//	{
//		unichar stack_buffer[64];
//		unichar * heap_buffer = NULL;
//		size_t strlen = [str length];
//		struct Scanner scanner;
//		enum Response result;
//		
//		if (strlen > 64) {
//			scanner.current = heap_buffer = malloc(strlen * sizeof(unichar));
//		} else {
//			scanner.current = stack_buffer;
//		}
//		scanner.end = scanner.current + strlen;
//		
//		[str getCharacters:scanner.current];
//		
//		prev(&scanner);
//		result = parseNumber(&scanner) ? token : notoken;
//		if (strlen != (scanner.end - scanner.current)) result = notoken;
//		
//		NSRange rangebase;
//		NSRange * range = &rangebase;
//		range->location = 0;
//		range->length = strlen - (scanner.end - scanner.current);
//		NSString * sidesc = subItems ? [[*subItems class] description] : @"null";
//		NSString * rdesc = range ? NSStringFromRange(*range) : @"null";
//		NSLog(@"%d := /New/tokenForString:[[\"%@\"]] forRange:%@ subItems:%@", result, str, rdesc, sidesc);
//	
//		if (heap_buffer) {
//			free(heap_buffer);
//		}
////		return result;
//	}
	{
		int result;
		if (isNumber(str)) {
			range->location = 0;
			range->length = [str length];
			result = token;
//			NSString * sidesc = subItems ? [[*subItems class] description] : @"null";
//			NSString * rdesc = range ? NSStringFromRange(*range) : @"null";
//			NSLog(@"%d := /New/tokenForString:[[\"%@\"]] forRange:%@ subItems:%@", result, str, rdesc, sidesc);
		} else {
			result = [super tokenForString:str forRange:range subItems:subItems];
//			NSString * sidesc = subItems ? [[*subItems class] description] : @"null";
//			NSString * rdesc = range ? NSStringFromRange(*range) : @"null";
//			NSLog(@"%d := /Leg/tokenForString:[[\"%@\"]] forRange:%@ subItems:%@", result, str, rdesc, sidesc);
		}
		return result;
	}
}

@end
