
// D for Xcode: Support tools for Xcode 3 scanners and base lexer
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

#import "DXScannerTools.h"

//#define UTF8_SCANNER

#ifdef UTF8_SCANNER
typedef char char_t;
#else
typedef unichar char_t;
#endif

struct Scanner {
	const char_t * current;
	const char_t * end;
};

inline char_t next(struct Scanner * scanner) {
	if (++scanner->current >= scanner->end) return 0;
	return *scanner->current;
}

inline char_t prev(struct Scanner * scanner) {
	--scanner->current;
	return *scanner->current;
}

inline BOOL atEnd(struct Scanner * scanner) {
	return scanner->current + 1 == scanner->end;
}

BOOL parseOneDigit(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '0'...'9':
	case '_':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOneHexDigit(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '0'...'9':
	case 'a'...'f':
	case 'A'...'F':
	case '_':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOneOctDigit(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '0'...'7':
	case '_':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOneBinDigit(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '0'...'1':
	case '_':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOneNonZeroDigit(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '1'...'9':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalDigits(struct Scanner * scanner) {
	while (parseOneDigit(scanner)) {
	}
	return TRUE;
}

BOOL parseOptionalHexDigits(struct Scanner * scanner) {
	while (parseOneHexDigit(scanner)) {
	}
	return TRUE;
}

BOOL parseOptionalOctDigits(struct Scanner * scanner) {
	while (parseOneOctDigit(scanner)) {
	}
	return TRUE;
}

BOOL parseOptionalBinDigits(struct Scanner * scanner) {
	while (parseOneBinDigit(scanner)) {
	}
	return TRUE;
}

BOOL parseDigits(struct Scanner * scanner) {
	return parseOneDigit(scanner) && parseOptionalDigits(scanner);
}

BOOL parseHexDigits(struct Scanner * scanner) {
	return parseOneHexDigit(scanner) && parseOptionalHexDigits(scanner);
}

BOOL parseOctDigits(struct Scanner * scanner) {
	return parseOneOctDigit(scanner) && parseOptionalOctDigits(scanner);
}

BOOL parseBinDigits(struct Scanner * scanner) {
	return parseOneBinDigit(scanner) && parseOptionalBinDigits(scanner);
}

BOOL parseOptionalSign(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '+':
	case '-':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalExponent(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'e':
	case 'E':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseHexPrefixSecondChar(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'x':
	case 'X':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseBinPrefixSecondChar(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'b':
	case 'B':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalIntegerSuffix(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'u':
	case 'U':
		switch (next(scanner)) {
		case 'L':
			return TRUE;
		}
		prev(scanner);
		return TRUE;
	case 'L':
		switch (next(scanner)) {
		case 'u':
		case 'U':
			return TRUE;
		}
		prev(scanner);
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalFloatSuffix(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'f':
	case 'F':
	case 'L':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalImaginarySuffix(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'i':
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalFloatExponent(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'e':
	case 'E':
		parseOptionalSign(scanner);
		parseOneDigit(scanner);
		parseOptionalDigits(scanner);
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseOptionalHexFloatExponent(struct Scanner * scanner) {
	switch (next(scanner)) {
	case 'p':
	case 'P':
		parseOptionalSign(scanner);
		parseOneDigit(scanner);
		parseOptionalDigits(scanner);
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseFractionalPart(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '.':
		parseOptionalDigits(scanner);
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseHexFractionalPart(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '.':
		parseOptionalHexDigits(scanner);
		return TRUE;
	}
	prev(scanner);
	return FALSE;
}

BOOL parseNumber(struct Scanner * scanner) {
	switch (next(scanner)) {
	case '0':
		if (parseHexPrefixSecondChar(scanner)) {
			parseOptionalHexDigits(scanner);
			if (parseHexFractionalPart(scanner)) {
				parseOptionalHexFloatExponent(scanner);
				parseOptionalFloatSuffix(scanner);
				parseOptionalImaginarySuffix(scanner);
				return TRUE;
			} else if (parseOptionalHexFloatExponent(scanner)) {
				parseOptionalFloatSuffix(scanner);
				parseOptionalImaginarySuffix(scanner);
				return TRUE;
			}
		} else if (parseBinPrefixSecondChar(scanner)) {
			parseOptionalBinDigits(scanner);
		} else if (parseFractionalPart(scanner)) {
			parseOptionalFloatExponent(scanner);
			parseOptionalFloatSuffix(scanner);
			parseOptionalImaginarySuffix(scanner);
			return TRUE;
		} else if (parseOptionalFloatExponent(scanner)) {
			parseOptionalFloatSuffix(scanner);
			parseOptionalImaginarySuffix(scanner);
			return TRUE;
		} else if (parseOneDigit(scanner)) {
			parseOptionalOctDigits(scanner);
		} else {
			// zero, alone, continue normally.
		}
		parseOptionalIntegerSuffix(scanner);
		parseOptionalImaginarySuffix(scanner);
		return TRUE;
	case '1'...'9':
		parseOptionalDigits(scanner);
		if (parseFractionalPart(scanner)) {
			parseOptionalFloatExponent(scanner);
			parseOptionalFloatSuffix(scanner);
		} else if (parseOptionalFloatExponent(scanner)) {
			parseOptionalFloatSuffix(scanner);
		} else {
			parseOptionalIntegerSuffix(scanner) || parseOptionalFloatSuffix(scanner);
		}
		parseOptionalImaginarySuffix(scanner);
		return TRUE;
	case '.':
		switch (next(scanner)) {
		case '0'...'9':
			parseOptionalDigits(scanner);
			parseOptionalFloatExponent(scanner);
			parseOptionalFloatSuffix(scanner);
			parseOptionalImaginarySuffix(scanner);
			return TRUE;
		}
		prev(scanner);
		break;
	}
	prev(scanner);
	return FALSE;
}

BOOL isNumber(NSString *str) {
	return numberLength(str) == [str length];
}

size_t numberLength(NSString *str) {
	size_t strlen = [str length];
	struct Scanner scanner;
	size_t result;
	
#ifdef UTF8_SCANNER
	scanner.current = [str UTF8String];
	scanner.end = scanner.current + strlen;
#else
	const size_t stackBufferLength = 64;
	unichar stackBuffer[stackBufferLength];
	unichar * buffer;
	if (strlen > stackBufferLength) {
		buffer = malloc(strlen * sizeof(unichar));
		if (!buffer) return 0; // not enough memory
	} else {
		buffer = stackBuffer;
	}
	[str getCharacters:buffer];
	scanner.current = buffer;
	scanner.end = scanner.current + strlen;
#endif
	
	prev(&scanner);
	if (parseNumber(&scanner))
		result = scanner.current + 1 - buffer;
	else
		result = 0;
	
#ifndef UTF8_SCANNER
	if (buffer != stackBuffer) free(buffer);
#endif
	
	return result;
}


size_t commentLength(NSString *str) {
	size_t pos = 0;
	const size_t length = [str length];
	if (pos < length && [str characterAtIndex:pos] != '/')
		return 0;
	
	if (++pos >= length) return 0;
	unichar secondChar = [str characterAtIndex:pos];
	if (secondChar == '/')
	{
		// single-line comment
		do if (++pos >= length) return length;
		while (pos < length && [str characterAtIndex:pos] != '\n' && [str characterAtIndex:pos] != '\r');
		return pos;
	}
	else if (secondChar == '*')
	{
		// block comment
		while (1)
		{
			if (++pos >= length) return length;
			unichar c = [str characterAtIndex:pos];
			if (c == '*' && pos+1 < length)
			{
				unichar c = [str characterAtIndex:pos+1];
				if (c == '/')
					return pos+2;
			}
		}
		assert(0);
	}
	else if (secondChar == '+')
	{
		// nested block comment
		size_t nesting = 1;
		while (nesting)
		{
			if (++pos >= length) return length;
			unichar c = [str characterAtIndex:pos];
			if (c == '+' && pos+1 < length)
			{
				unichar c = [str characterAtIndex:pos+1];
				if (c == '/')
					--nesting, ++pos;
			}
			else if (c == '/' && pos+1 < length)
			{
				unichar c = [str characterAtIndex:pos+1];
				if (c == '+')
					++nesting, ++pos;
			}
		}
		return pos+1;
	}
	else
		return 0;
}

size_t stringLength(NSString *str) {
	size_t pos = 0;
	const size_t length = [str length];
	if (length == 0)
		return 0;
	
	switch ([str characterAtIndex:pos])
	{
		case '"':
			while (1) {
				if (++pos >= length) return length;
				if ([str characterAtIndex:pos] == '"')
					break;
				if ([str characterAtIndex:pos] == '\\')
					if (++pos >= length) return length;;
			}
			break;
		case '\'':
			while (1) {
				if (++pos >= length) return length;
				if ([str characterAtIndex:pos] == '\'')
					break;
				if ([str characterAtIndex:pos] == '\\')
					if (++pos >= length) return length;;
			}
			break;
		case 'r': case 'x':
			if (++pos >= length) return 0;
			if ([str characterAtIndex:pos] != '"')
				return 0;
			do 
				if (++pos >= length) return length;
			while ([str characterAtIndex:pos] != '"');
			break;
		case '`':
			do 
				if (++pos >= length) return length;
			while ([str characterAtIndex:pos] != '`');
			break;
		case 'q':
			if (++pos >= length) return 0;
			if ([str characterAtIndex:pos] == '"')
			{
				if (++pos >= length) return length;
				
				unichar openingMarker = [str characterAtIndex:pos];
				unichar closingMarker = 0;
				BOOL nesting = NO;
				NSString *closingString = nil;
				switch (openingMarker) {
					case '[': closingMarker = ']'; nesting = YES; break;
					case '(': closingMarker = ')'; nesting = YES; break;
					case '<': closingMarker = '>'; nesting = YES; break;
					case '{': closingMarker = '}'; nesting = YES; break;
					case 'a'...'z': case 'A'...'Z':
					{
						NSRange range = NSMakeRange(pos, 0);
						do if (++pos >= length) return length;
						while ([str characterAtIndex:pos] != '\n');
						range.length = pos - range.location;
						closingString = [NSString stringWithFormat:@"\n%@\"", [str substringWithRange:range]];
						break;
					}
					case ' ': case '\t': case '\n': case '\r':
						return pos;
					default: closingMarker = openingMarker; break;
				}
				
				if (nesting) {
					size_t nesting = 1;
					do {
						if (++pos >= length) return length;
						
						if ([str characterAtIndex:pos] == openingMarker)
							++nesting;
						else if ([str characterAtIndex:pos] == closingMarker)
							--nesting;
					} while (nesting > 0);
					if (++pos >= length) return length; // skip marker
					if ([str characterAtIndex:pos] != '"') {
						return pos;
					}
				} else {
					if (closingString == nil)
						closingString = [NSString stringWithCharacters:&closingMarker length:1];
					
					if (++pos >= length) return length;
					NSRange foundRange = [str rangeOfString:closingString options:NSLiteralSearch range:NSMakeRange(pos, length-pos)];
					if (foundRange.length == 0)
						return length;
				
					pos = foundRange.location + foundRange.length - 1;
					if (pos >= length) return length;
					
					if ([closingString length] == 1)
					{
						// delimiter doesn't include quote, check for quote
						if (++pos >= length) return length; // skip delimiter
						if ([str characterAtIndex:pos] != '"') {
							return pos;
						}
					}
				}
			}
			else if ([str characterAtIndex:pos] == '{')
			{
				size_t nesting = 1;
				do {
					if (++pos >= length) return length;
					
					switch ([str characterAtIndex:pos])
					{
						case '{': ++nesting; break;
						case '}': --nesting; break;
					}
				} while (nesting > 0);
			}
			else
				return 0;
			break;
		default:
			return 0;
	}
	
	if (++pos >= length) return length;
	switch ([str characterAtIndex:pos]) {
		case 'c': case 'w': case 'd':
			++pos;
			break;
		default:
			break;
	}
	return pos;
}