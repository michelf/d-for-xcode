
// D for Xcode: Source Lexer
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

#import <XCPSourceParsing.h>


/*!
 * Lexer class used by Xcode for syntax highlighting.
 */
@interface DXDSourceLexer : PBXSourceLexer {
	NSString *stringSource;
	NSSet *keywords;
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
- (void)scanForLinksBetweenStart:(const unsigned char *)start andEnd:(const unsigned char *)end startingAt:(unsigned int)pos;

/*!
 * Scan ddoc documentation comments for keywords (section markers) and call 
 * method gotDocKeywordForRange: for each of them.
 */
- (void)scanForDocKeywordsBetweenStart:(const unsigned char *)start andEnd:(const unsigned char *)end startingAt:(unsigned int)pos;

@end
