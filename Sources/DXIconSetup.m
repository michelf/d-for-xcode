
// D for Xcode: Icon Setup
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

#import "DXIconSetup.h"
#import <Cocoa/Cocoa.h>

// Retain images here for the Objective-C 2.0 garbage collector.
NSMutableArray *iconKeeper;

@implementation DXIconSetup

+ (void)load {
	// Initialize icons.
	NSString *dir = [[NSBundle bundleForClass:[self class]] resourcePath];
	NSImage *img;
	NSString *path;
	unsigned int index = 0;
	
	NSArray *iconList = [NSArray arrayWithObjects:
		@"PBX-d-Icon", @"PBX-d-small-Icon",
		@"PBX-di-Icon", @"PBX-di-small-Icon", 
		@"DX-option-language-d", nil];
	iconKeeper = [[NSMutableArray alloc] init];
	
	NSEnumerator *e = [iconList objectEnumerator];
	NSString *name;
	while (name = [e nextObject]) {
		path = [NSString stringWithFormat:@"%@/%@.png", dir, name];
		img = [[NSImage alloc] initByReferencingFile:path];
		[img setName:name];
		[iconKeeper addObject:img];
		[img release];
	}
}

@end
