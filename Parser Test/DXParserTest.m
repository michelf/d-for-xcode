
// D parser function tests
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

// Use this test program to diagnose problems occuring with specific source
// files. Tests dependency retrieval and the source scanner for the file
// path given as first argument.


#import <Foundation/Foundation.h>

#import "DXParserTools.h"
#import "XCPSourceParsing.h"


int main(unsigned int argc, char **argv) {
	if (argc < 2) {
		NSLog(@"Missing test file path as first argument.");
		return 0;
	}
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSString *path = [NSString stringWithCString:argv[1]];
	NSString *source = [NSString stringWithContentsOfFile:path];
	NSLog(@"%d characters in \"%@\"", [source length], path);
	
	DXParserInit();
	
	NSLog(@"--- Dependencies ---");
	NSSet *dependencies = DXSourceDependenciesForPath(source);
	NSEnumerator *e = [dependencies objectEnumerator];
	id filepath;
	while (filepath = [e nextObject]) {
		NSLog(filepath);
	}
	
	NSLog(@"--- Source Scanner ---");
	id scan = DXScanSource(source);
	int i, end = [scan numberOfChildren];
	for (i = 0; i < end; ++i) {
		id child = [scan childAtIndex:i];
		NSLog(@"%d %@ nameRange:%@ range:%@", [child type], [child description], NSStringFromRange([child nameRange]), NSStringFromRange([child range]));
	}
	
	[pool release];
	return 0;
}
