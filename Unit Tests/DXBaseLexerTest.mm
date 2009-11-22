//
//  DXBaseLexer.m
//  D for Xcode
//
//  Created by Michel Fortin on 2009-10-11.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

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
