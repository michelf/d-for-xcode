//
//  DXParserTools.m
//  D for Xcode
//
//  Created by Michel Fortin on 2007-09-29.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "DXParserTools.h"
#import "DXDSourceScannerItem.h"

#include <string>

#undef check
#define Ptr DMD_Ptr
#include "parse.h"
#include "import.h"
#include "identifier.h"
#include "declaration.h"
#include "module.h"


NSSet *DXSourceDependenciesForPath(NSString *source) {
	NSMutableSet *files = [[NSMutableSet alloc] init];

	// Need a null-terminated UTF-8 string.
	const char *bytes = [source UTF8String];
	
	if (bytes) {
		size_t length = strlen(bytes);
	
		// This code is a simple parser that only catches imported modules 
		// names,  and translate them into file names. It tokenizes the file 
		// using the lexer from the DMD front end.
		
		try {
			Lexer lexer(NULL, (unsigned char *)bytes, 0, length, 0, 0);
			TOK token;
			
			FIND_IMPORT:
			while (1) {
				token = lexer.nextToken();
				if (token == TOKeof) goto END;
				if (token != TOKimport) goto FIND_IMPORT;
				
				// Set first part of file name.
				std::string filename;
				
				while (1) {
					token = lexer.nextToken();
					if (token == TOKeof) goto END;
					if (token == TOKlparen) {
						// Parse file import: import("file.txt")
						token = lexer.nextToken();
						if (token == TOKstring) {
							char *str = (char *)lexer.token.ustring;
							
							token = lexer.nextToken();
							if (token == TOKrparen) {
								[files addObject:[NSString stringWithUTF8String:str]];
							}
						}
						goto FIND_IMPORT;
					}
					if (token != TOKidentifier) goto FIND_IMPORT;
					
					filename += lexer.token.toChars();
					
					token = lexer.nextToken();
					switch (token) {
						case TOKdot:
							// Add new part to filename.
							filename += "/";
							break;
						case TOKcolon:
							// Skip over module bindings.
							while (1) {
								token = lexer.nextToken();
								switch (token) {
									case TOKsemicolon:
									case TOKcomma:
										goto END_BINDINGS;
									case TOKeof:
										goto END;
								}
							}
							END_BINDINGS:
							// Fall through semicolon processing.
						case TOKsemicolon:
						case TOKcomma:
							// Add file extension to filename, add filename to list.
							filename += ".d";
							[files addObject:[NSString stringWithUTF8String:filename.c_str()]];
							filename += "i"; // .di
							[files addObject:[NSString stringWithUTF8String:filename.c_str()]];

							if (token == TOKcomma) filename = ""; // Read new module.
							else                   goto FIND_IMPORT; // Search new import.
							break;
						case TOKassign:
							// Skip current filename as it represents the alias
							// and restart reading module name.
							filename = "";
							break;
						default:
							// Search for a new import
							goto FIND_IMPORT;
							break;
					}
				}
			}
		} catch (...) {
			/* lexer "crashed", continue normally */
		}
	}
	END:
	
	return [files autorelease];
}


PBXSourceScannerItem *DXScanSource(NSString *code) {
	// Need a null-terminated UTF-8 string.
	const char *bytes = [code UTF8String];
	Identifier ident("__module", TOKidentifier);
	Module module("__module", &ident, 0, 0);
	
	if (bytes) {
		try {
			size_t length = strlen(bytes);
		
			Parser parser(&module, (unsigned char *)bytes, length, 0);
			parser.nextToken(); // Start up parsing.
			Array *array = parser.parseModule();
			
			PBXSourceScannerItem *root = [[[PBXSourceScannerItem alloc] initWithName:@"module" type:DXTypeRoot] autorelease];
			[root setRange:NSMakeRange(0, [code length])];
			
			DXPopulateItemWithSymbols(root, array);
			
			return root;
		} catch (...) {
			/* parser "crashed", continue normally */
		}
	}
	
	return nil;
}
