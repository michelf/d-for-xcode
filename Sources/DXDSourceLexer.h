//
//  DXDSourceLexer.h
//  D for Xcode
//
//  Created by Michel Fortin on 2007-09-29.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <XCPSourceParsing.h>


/*!
 * Lexer class used by Xcode for syntax highlighting.
 */
@interface DXDSourceLexer : PBXSourceLexer {
	NSString *stringSource;
}

/*!
 * Create new lexer with the lexical rules given in the .pblangspec file.
 */
- (id)initWithLexicalRules:(PBXLexicalRules *)rules;

/*!
 * Sets the source string *by reference*. The string is externally changed 
 * when the user edit the source.
 */
- (void)setString:(NSString *)code range:(NSRange)range;

/*!
 * Tell that a portion of the source string needs to be reparsed.
 */
- (void)scanSubRange:(NSRange)range startingInState:(int)state;


// Helper methods specific to this implementation

/*!
 * Scan for links within a comments and call method gotLinkForRange: or 
 * gotMailAddressForRange: for each of them.
 */
- (void)scanForLinksBetweenStart:(unsigned char *)start andEnd:(unsigned char *)end startingAt:(unsigned int)pos;

/*!
 * Scan ddoc documentation comments for keywords (section markers) and call 
 * method gotDocKeywordForRange: for each of them.
 */
- (void)scanForDocKeywordsBetweenStart:(unsigned char *)start andEnd:(unsigned char *)end startingAt:(unsigned int)pos;

@end
