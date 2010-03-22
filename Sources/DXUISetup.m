
// D for Xcode: UI-only code loader to play nice with command line tool
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

#import "DXUISetup.h"


@implementation DXUISetup

+ (void)load {
	if (objc_lookUpClass("XCRegExScanner")) {
		// Source scanning primitives present, can load dependent classes
		NSString *path = [[NSBundle bundleForClass:[self class]] resourcePath];
		path = [path stringByAppendingPathComponent:@"D for Xcode UI.bundle"];
		[[NSBundle bundleWithPath:path] load];
		//NSLog(@"Loading D fox Xcode UI components %@", path);
	}
	else {
		// We're likely loaded inside xcodebuild command line tool, don't need
		// source scanner
		//NSLog(@"Loading D fox Xcode UI components");
	}

}

@end
