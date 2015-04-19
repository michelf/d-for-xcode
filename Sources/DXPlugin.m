
// D for Xcode: UI-only code loader to play nice with command line tool
// Copyright (c) 2007-2011  Michel Fortin
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

#import "DXPlugin.h"


@implementation DXPlugin

+ (BOOL)pluginAttemptLoadingUI:(NSNotification *)notification
{
	NSLog(@"Notification: %@", notification);
	if (objc_lookUpClass("DVTSourceScanner"))
	{
		// Source scanning primitives present, can load dependent classes
		NSString *path = [[NSBundle bundleForClass:[self class]] resourcePath];
		path = [path stringByAppendingPathComponent:@"D for Xcode UI.bundle"];
		NSLog(@"Loading D fox Xcode UI components from %@", path);
		[[NSNotificationCenter defaultCenter] removeObserver:self name:NSBundleDidLoadNotification object:nil];
		[[NSBundle bundleWithPath:path] load];
		return YES;
	}
	else {
		// We're likely loaded inside xcodebuild command line tool, 
		// can't load source scanners and UI stuff
		//NSLog(@"NOT loading D fox Xcode UI components !!");
		return NO;
	}
}

+ (void)pluginDidLoad:(NSBundle *)plugin
{
	if ([self pluginAttemptLoadingUI:nil] == NO)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pluginAttemptLoadingUI:) name:NSBundleDidLoadNotification object:nil];
	}
}

@end
