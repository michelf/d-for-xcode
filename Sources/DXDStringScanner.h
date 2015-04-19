
// D for Xcode: String Scanner for Xcode 3
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

#import <Cocoa/Cocoa.h>
#import <DVTFoundation/DVTSourceScanner.h>
#import <DVTFoundation/DVTCharStream.h>

@interface DXDStringScanner : DVTSourceScanner {
}

- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(DVTCharStream *)fp20 range:(NSRange)fp24 dirtyRange:(NSRange *)fp32;

- (BOOL)tokenStringOnly;

@end

@interface DXDTokenStringScanner : DXDStringScanner {
}

- (BOOL)tokenStringOnly;

@end
