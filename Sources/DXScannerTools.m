
// D for Xcode: Support tools for special scanners for Xcode 3
// Copyright (C) 2007  Michel Fortin
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

typedef char char_t;

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
		case 'l':
		case 'L':
			return TRUE;
		}
		prev(scanner);
		return TRUE;
	case 'l':
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
	unichar stack_buffer[64];
	unichar * heap_buffer = NULL;
	size_t strlen = [str length];
	struct Scanner scanner;
	BOOL result;
	
//	if (strlen > 64) {
//		scanner.current = heap_buffer = malloc(strlen * sizeof(unichar));
//	} else {
//		scanner.current = stack_buffer;
//	}
	scanner.current = [str UTF8String];
	scanner.end = scanner.current + strlen;
	
//	[str getCharacters:scanner.current];
	
	prev(&scanner);
	result = parseNumber(&scanner);
	if (result && !atEnd(&scanner)) result = NO;
	
	return result;
}

