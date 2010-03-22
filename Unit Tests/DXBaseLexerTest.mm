
// Unit tests for base lexer
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

#import "DXBaseLexerTest.h"
#import "DXBaseLexer.h"

@implementation DXBaseLexerTest

- (void)testVariousTokens {
	NSString *input = 
		@"import a.b : e.f;"
		"/+ hello /+ hello +/ hello +/"
		" \"test\" "
		" r\"test\" "
		" `test` "
		" 123.556 ";
	
	DXBaseLexer lexer(input);
	
	STAssertEquals(lexer.nextToken(), TOKidentifier, @"import token expected");
	STAssertEquals(lexer.nextToken(), TOKidentifier, @"identifier token expected");
	STAssertEquals(lexer.nextToken(), TOKdot, @"dot token expected");
	STAssertEquals(lexer.nextToken(), TOKidentifier, @"identifier token expected");
	STAssertEquals(lexer.nextToken(), TOKcolon, @"colon token expected");
	STAssertEquals(lexer.nextToken(), TOKidentifier, @"identifier token expected");
	STAssertEquals(lexer.nextToken(), TOKdot, @"dot token expected");
	STAssertEquals(lexer.nextToken(), TOKidentifier, @"identifier token expected");
	STAssertEquals(lexer.nextToken(), TOKsemicolon, @"semicolon token expected");
	
	STAssertEquals(lexer.nextToken(), TOKcomment, @"comment token expected");
	STAssertEquals(lexer.nextToken(), TOKstring, @"string token expected");
	STAssertEquals(lexer.nextToken(), TOKstring, @"string token expected");
	STAssertEquals(lexer.nextToken(), TOKstring, @"string token expected");
	STAssertEquals(lexer.nextToken(), TOKnumber, @"number token expected");
	
	STAssertEquals(lexer.nextToken(), TOKeof, @"eof token expected");
}

@end
