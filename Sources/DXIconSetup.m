
// D for Xcode: Icon Setup
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

#import "DXIconSetup.h"
#import <Cocoa/Cocoa.h>

// Retain images here for the Objective-C 2.0 garbage collector.
NSMutableArray *iconKeeper;

static BOOL DXIsXcode31OrLater() {
	const int xcode3version = 921;
	int version = [[[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"] intValue];
	return version > xcode3version || version == 0;
}

static void DXRegisterIcon(NSString *path, NSString *name) {
	if (iconKeeper == nil) {
		iconKeeper = [[NSMutableArray alloc] init];
	}
	NSImage *img = [[NSImage alloc] initByReferencingFile:path];
	[img setName:name];
	[iconKeeper addObject:img];
	[img release];
}

@implementation DXIconSetup

+ (void)load {
	// Initialize icons.
	NSString *dir = [[NSBundle bundleForClass:[self class]] resourcePath];
	NSString *path;
	unsigned int index = 0;
	
	NSArray *iconList = [NSArray arrayWithObjects:
		@"PBX-d-small-Icon",
		@"PBX-di-small-Icon", 
		@"DX-option-language-d", nil];
	// The following icons come in two versions: one for Xcode 3.1 and later
	// and another for previous versions of Xcode, suffixed with "-old".
	NSArray *versionnedIconList = [NSArray arrayWithObjects:
		@"PBX-d-Icon", 
		@"PBX-di-Icon", nil];
	NSEnumerator *e;
	NSString *name;
	
	e = [iconList objectEnumerator];
	while (name = [e nextObject]) {
		path = [NSString stringWithFormat:@"%@/%@.png", dir, name];
		DXRegisterIcon(path, name);
	}
	
	e = [versionnedIconList objectEnumerator];
	NSString *suffix = DXIsXcode31OrLater() ? @"" : @"-old";
	while (name = [e nextObject]) {
		path = [NSString stringWithFormat:@"%@/%@%@.png", dir, name, suffix];
		DXRegisterIcon(path, name);
	}
}

@end
