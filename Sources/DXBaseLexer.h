
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

#import <Foundation/Foundation.h>
#import "XCPSourceParsing.h"

enum TOK {
	TOKeof, TOKimport, TOKidentifier, TOKstring, TOKunknown, TOKnumber,	
	TOKchar, TOKcomment, TOKinvalid,
	TOKdot = '.',
	TOKlparen = '(',
	TOKrparen = ')',
	TOKsemicolon = ';',
	TOKcolon = ':',
	TOKcomma = ',',
	TOKassign = '=',
};

class DXBaseLexer {
public:
	DXBaseLexer(NSString *aSource);
	
	TOK skipToImport();
	TOK nextToken(bool skipComments = false);
	
	NSRange tokenRange;
	NSRange valueRange() const;
	NSString *stringValue() const;
		
private:
	NSString *source;
	size_t length;
	size_t pos;
	NSCharacterSet *whitespace;
	NSCharacterSet *alphanum;
	NSCharacterSet *numeric;
	NSCharacterSet *numericContent;
	
	size_t valueType;
};
