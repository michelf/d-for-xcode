
// D for Xcode: Dependency Scanner
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

#import "DXDependencyScanner.h"
#import "DXBaseLexer.h"

#include <string>


NSSet *DXSourceDependenciesForPath(NSString *source) {
	NSMutableSet *files = [[NSMutableSet alloc] init];

	size_t length = [source length];
	NSRange searchRange = NSMakeRange(0, length);
	
	// This code is a simple parser that only catches imported modules 
	// names, and translate them into file names. It tokenizes the file 
	// using a custom simplified lexer.
	
	DXBaseLexer lexer(source);
	TOK token;
			
	FIND_IMPORT:
	while (1) {
		token = lexer.skipToImport();
		if (token == TOKeof) goto END;
		if (token != TOKimport) goto FIND_IMPORT;
		
		// Set first part of file name.
		NSMutableString *filename = [NSMutableString string];
		
		while (1) {
			token = lexer.nextToken(true);
			if (token == TOKeof) goto END;
			if (token == TOKlparen) {
				// Parse file import: import("file.txt")
				token = lexer.nextToken(true);
				if (token == TOKstring) {
					NSString *stringValue = lexer.stringValue();
					
					token = lexer.nextToken(true);
					if (token == TOKrparen) {
						[files addObject:stringValue];
					}
				}
				goto FIND_IMPORT;
			}
			if (token != TOKidentifier) goto FIND_IMPORT;
			
			[filename appendString:lexer.stringValue()];
			
			token = lexer.nextToken(true);
			switch (token) {
				case TOKdot:
					// Add new part to filename.
					[filename appendString:@"/"];
					break;
				case TOKcolon:
					// Skip over module bindings.
					while (1) {
						token = lexer.nextToken(true);
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
					[filename appendString:@".d"];
					[files addObject:[filename copy]];
					[filename appendString:@"i"]; // .di
					[files addObject:[filename copy]];

					if (token == TOKcomma) [filename deleteCharactersInRange:NSMakeRange(0, [filename length])]; // Read new module.
					else                   goto FIND_IMPORT; // Search new import.
					break;
				case TOKassign:
					// Skip current filename as it represents the alias
					// and restart reading module name.
					[filename deleteCharactersInRange:NSMakeRange(0, [filename length])];
					break;
				default:
					// Search for a new import
					goto FIND_IMPORT;
					break;
			}
		}
	}
		
	END:
	return [files autorelease];
}
