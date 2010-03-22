
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

#import "DXDSourceLexer.h"
#import "DXDependencyScanner.h"
#import "DXBaseLexer.h"

@implementation DXDSourceLexer

- (id)initWithLexicalRules:(PBXLexicalRules *)rules {
	// This implementation does not do anything with the lexical rules, but
	// still pass them to the superclass for initialization.
 
	[super initWithLexicalRules:rules];
	// ignore rules, use the DMD Front End lexer instead.
	stringSource = [[NSString alloc] init];
	return self;
}

- (void)setString:(NSString *)code range:(NSRange)range {
	[stringSource release];
	stringSource = [code retain];
}

- (void)scanSubRange:(NSRange)range startingInState:(int)state {
	// Note: This implementation always reparse the whole string.

	// Need a null-terminated UTF-8 string.
	const char *bytes = [stringSource UTF8String];
	if (!bytes) return;
	
	size_t length = strlen(bytes);
	unsigned int maxLoc = [stringSource length];
		
	// Clean previous coloring.
	[self gotIdentifierForRange:NSMakeRange(0, [stringSource length])];
	
	try {
		DXBaseLexer lexer(stringSource);
		TOK token;
		while ((token = lexer.nextToken()) != TOKeof) {
			NSRange tokenRange = lexer.tokenRange;
			if (tokenRange.location > maxLoc) {
				tokenRange.location = maxLoc;
			}
			if (tokenRange.length > maxLoc-tokenRange.location) {
				tokenRange.length = maxLoc-tokenRange.location;
			}
			switch (token) {
				case TOKnumber:
					[self gotNumberForRange:tokenRange];
					break;
				case TOKchar:
					[self gotCharacterForRange:tokenRange];
					break;
				case TOKstring:
					[self gotStringForRange:tokenRange];
					break;
				case TOKcomment:
				{
					// Test for documentation comments (doubled second character).
					NSString *str = lexer.stringValue();
					NSData *data = [str dataUsingEncoding:NSUTF8StringEncoding];
					const unsigned char *ptr = (const unsigned char *)[data bytes];
					const unsigned char *endPtr = ptr+[data length];
					
					if (tokenRange.length >= (*(ptr+1) == '/' ? 3 : 5) &&
						*(ptr+1) == *(ptr+2))
					{
						[self gotDocCommentForRange:tokenRange];
						// Scan for keywords, skipping first character.
						[self scanForDocKeywordsBetweenStart:ptr+1 andEnd:endPtr startingAt:tokenRange.location+1];
					} else {
						[self gotCommentForRange:tokenRange];
					}
					[self scanForLinksBetweenStart:ptr andEnd:endPtr startingAt:tokenRange.location];
					break;
				}
				case TOKidentifier:
					[[self rules] isKeyword:lexer.stringValue()];
					break;
				default:
					break;
			}
		}
	} catch (...) {
		/* lexer "crashed", abort. */
	}
}

- (void)scanForLinksBetweenStart:(const unsigned char *)start andEnd:(const unsigned char *)end startingAt:(unsigned int)pos {
	// Scan for URLs in comment.
	const unsigned char *current = start;
	while (++current < end) {
		++pos;
		switch (*current) {
			case 'a'...'z':
			case 'A'...'Z':
			{
				unsigned int linkStart = pos;
				unsigned int linkEnd = pos;
				while (++current < end) {
					++pos;
					switch (*current) {
					case 'a'...'z':
					case 'A'...'Z':
					case '0'...'9':
					case '.': case '-': case '_':
						continue; // continue loop.
					case '@':
						// Email.
						while (++current < end) {
							++pos;
							switch (*current) {
								case 'a'...'z':
								case 'A'...'Z':
								case '0'...'9':
								case '.': case '-': case '_':
								case '%':
									linkEnd = pos+1;
									continue; // continue loop.
								default:
									break;
							}
							break; // break loop.
						}
						if (linkEnd != linkStart) {
							NSRange linkRange = NSMakeRange(linkStart, linkEnd-linkStart);
							[self gotMailAddressForRange:linkRange];
							linkStart = linkEnd;
						}
						break; // break switch, break loop, search again.
					case ':':
						// General URL.
						if (current+2 < end && 
							*(current+1) == '/' && *(current+2) == '/')
						{
							current += 2;
							pos += 2;
							
							unsigned int stakedParen = 0;
							
							while (++current < end) {
								++pos;
								switch (*current) {
									case 'a'...'z':
									case 'A'...'Z':
									case '0'...'9':
									case '-': case '_': case '/':
									case '+': case '=': case '~':
										linkEnd = pos+1;
										continue; // continue loop.
									case '.': case '%': case '?': case '!': 
									case '@': case '&': case '#': case ';':
									case ',': case '\'': case ':':
										// Be more inteligent than Xcode:
										// allow these in URLs, but not at the end.
										continue;
									case '(':
										// Be more inteligent than Xcode:
										// count open parenthesis.
										stakedParen++;
										linkEnd = pos+1;
										continue; // continue loop.
									case ')':
										// Be more inteligent than Xcode:
										// allow at the end only if a parenthesis 
										// is open.
										if (stakedParen) {
											stakedParen--;
											linkEnd = pos+1;
										}
										continue; // continue loop.
									default:
										break;
								}
								break; // break loop.
							}
							if (linkEnd != linkStart) {
								NSRange linkRange = NSMakeRange(linkStart, linkEnd-linkStart);
								[self gotURLForRange:linkRange];
								linkStart = linkEnd;
							}
						}
						break; // break switch, break loop, search again.
					}
					break; // break loop, search again.
				}
				break;
			}
			case 0xC0 ... 0xFF:
				// Multibyte UTF-8 char: skip all bytes until end of character.
				while ((*(current+1) & 0xC0) == 0x80)  ++current;
				break;
		}
		// search next link.
	}
}

- (void)scanForDocKeywordsBetweenStart:(const unsigned char *)start andEnd:(const unsigned char *)end startingAt:(unsigned int)pos {
	const unsigned char *current = start;
	bool waitingMarginChar = false;
	goto START;
	
	while (++current < end) {
		++pos;
		switch (*current) {
		case '\n':
		START:
			waitingMarginChar = true;
			while (++current < end) {
				++pos;
				switch (*current) {
					case ' ': case '\t':
						continue; // skip leading whitespace, continue loop.
					case '*': case '+': case '/':
						if (waitingMarginChar && *current == *start) {
							waitingMarginChar = false;
							continue; // skip margin character, continue loop.
						}
						break; // restart new keyword.
					case 'a'...'z':
					case 'A'...'Z':
					{
						unsigned int keywordStart = pos;
						while (++current < end) {
							++pos;
							switch (*current) {
							case 'a'...'z':
							case 'A'...'Z':
							case '0'...'9':
							case '_':
								continue; // continue loop.
							case ':':
								{
									unsigned int length = pos-keywordStart+1;
									NSRange keywordRange = NSMakeRange(keywordStart, length);
									[self gotDocCommentKeywordForRange:keywordRange];
								}
								break;
							default:
								break;
							}
							break; // break loop.
						}
						break; // break switch.
					}
					default:
						--pos; --current; // Reprocess character in main loop.
						break; // break switch.
				}
				break; // break loop, search for next keyword.
			}
			break;
		case 0xC0 ... 0xFF:
			// Multibyte UTF-8 char: skip all bytes until end of character.
			while ((*(current+1) & 0xC0) == 0x80)  ++current;
			break;
		}
		// search next link.
	}
}

- (void)dealloc {
	[stringSource release];
	[super dealloc];
}

@end
