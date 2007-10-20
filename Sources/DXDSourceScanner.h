//
//  DXDSourceScanner.h
//  D for Xcode
//
//  Created by Michel Fortin on 2007-09-29.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <XCPSourceParsing.h>
#import <XCPSpecifications.h>
#import <DXDSourceLexer.h>


/*!
 * Source code scanner class used by Xcode to create the editor's symbol menu.
 */
@interface DXDSourceScanner : PBXSourceScanner {
}

/*!
 * Parse a string and return the root PBXSourceScannerItem for the document.
 */
- (PBXSourceScannerItem *)parseString:(NSString *)code;

@end
