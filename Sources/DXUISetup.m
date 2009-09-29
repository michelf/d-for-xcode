//
//  DXUISetup.m
//  D for Xcode
//
//  Created by Michel Fortin on 2009-09-28.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "DXUISetup.h"


@implementation DXUISetup

+ (void)load {
	if (objc_lookUpClass("XCRegExScanner")) {
		// Source scanning primitives present, can load dependent classes
		NSString *path = [[NSBundle bundleForClass:[self class]] resourcePath];
		path = [path stringByAppendingPathComponent:@"D for Xcode UI.bundle"];
		[[NSBundle bundleWithPath:path] load];
		NSLog(@"Loading D fox Xcode UI components %@", path);
	}
	else {
		// We're likely loaded inside xcodebuild command line tool, don't need
		// source scanner
		NSLog(@"Loading D fox Xcode UI components");
	}

}

@end
