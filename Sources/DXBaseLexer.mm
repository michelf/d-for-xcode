
// D for Xcode: Lexer for basic parsing of D files
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

#import "DXBaseLexer.h"
#import "DXScannerTools.h"

DXBaseLexer::DXBaseLexer(NSString *aSource) {
	tokenRange = NSMakeRange(0, 0);
	source = aSource;
	length = [source length];
	pos = 0;
	whitespace = [NSCharacterSet whitespaceAndNewlineCharacterSet];
	alphanum = [NSCharacterSet alphanumericCharacterSet];
	numeric = [NSCharacterSet decimalDigitCharacterSet];
	numericContent = [NSCharacterSet characterSetWithCharactersInString:@"0123456789abcdefghijklmnopqrstuvwxyz_.-+"];
}

TOK DXBaseLexer::skipToImport() {
	NSRange searchRange = NSMakeRange(pos, length - pos);
	NSRange importRange = [source rangeOfString:@"import" options:NSLiteralSearch range:searchRange];
	pos = importRange.location + importRange.length;
	
	if (importRange.length == 0 || pos >= length)
		return TOKeof;
	
	if (importRange.location != 0 && 
		[alphanum characterIsMember:[source characterAtIndex:importRange.location-1]])
		return TOKeof; // import is in the middle of a word.
	if ([alphanum characterIsMember:[source characterAtIndex:pos]])
		return TOKeof; // import is just a prefix.
	
	return TOKimport;
}

TOK DXBaseLexer::nextToken(bool skipComments) {
	BEGINING:
	while (1) {
		if (pos >= length)
			return TOKeof;
		if (![whitespace characterIsMember:[source characterAtIndex:pos]])
			break;
		++pos;
	}
	
	unichar c = [source characterAtIndex:pos];
	
	tokenRange.location = pos;
	if (c == '"' || c == '`') {
		tokenRange.length = stringLength([source substringFromIndex:pos]);
		pos += tokenRange.length;
		valueType = c;
		return TOKstring;
	}
	else if ((c == 'r' || c == 'q' || c == 'x') && (tokenRange.length = stringLength([source substringFromIndex:pos])) != 0) {
		pos += tokenRange.length;
		valueType = c;
		return TOKstring;
	}
	else if (c == '\'' && (tokenRange.length = stringLength([source substringFromIndex:pos])) != 0) {
		pos += tokenRange.length;
		valueType = c;
		return TOKchar;
	}
	else if (c == '/' && (tokenRange.length = commentLength([source substringFromIndex:pos])) != 0) {
		pos += tokenRange.length;
		valueType = c;
		if (skipComments) goto BEGINING;
		return TOKcomment;
	}
	else if ([numeric characterIsMember:c]) {
		NUMERIC:
		while (1) {
			c = [source characterAtIndex:pos];
			if (pos >= length || !([alphanum characterIsMember:c] || c == '.')) {
				tokenRange.length = pos - tokenRange.location;
				valueType = 0;
				if (!isNumber([source substringWithRange:tokenRange]))
					return TOKinvalid;
				return TOKnumber;
			}
			++pos;
		}
	}
	else if ([alphanum characterIsMember:c]) {
		while (1) {
			if (pos >= length || ![alphanum characterIsMember:[source characterAtIndex:pos]]) {
				tokenRange.length = pos - tokenRange.location;
				valueType = 0;
				return TOKidentifier;
			}
			++pos;
		}
	}
	else {
		tokenRange.length = 1;
		pos += tokenRange.length;
		switch (c) {
			case '.':
				if (pos < length && [numeric characterIsMember:[source characterAtIndex:pos]])
					goto NUMERIC;
				return TOKdot;
			case '(': return TOKlparen;
			case ')': return TOKrparen;
			case ';': return TOKsemicolon;
			case ':': return TOKcolon;
			case ',': return TOKcomma;
			case '=': return TOKassign;
			default:  return TOKunknown;
		}
	}
}

NSRange DXBaseLexer::valueRange() const {
	switch (valueType) {
		case '"': case '`':
			// remove quotes from range
			return NSMakeRange(tokenRange.location+1, tokenRange.location-2);
		case 'r': case 'q': case 'x':
			return NSMakeRange(tokenRange.location+2, tokenRange.location-3);
		default:
			return tokenRange;
	}
}

NSString *DXBaseLexer::stringValue() const {
	// TODO: process string content depending on valueType.
	return [source substringWithRange:valueRange()];
}
