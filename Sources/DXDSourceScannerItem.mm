//
//  DXDSourceScannerItems.m
//  D for Xcode
//
//  Created by Michel Fortin on 2007-10-02.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "DXDSourceScannerItem.h"

#undef check
#define Ptr DMD_Ptr
#include "parse.h"
#include "import.h"
#include "identifier.h"
#include "declaration.h"
#include "aggregate.h"
#include "attrib.h"

void DXPopulateItemWithSymbols(PBXSourceScannerItem *item, Array *symbols) {
	if (symbols == NULL) return;
	
	for (int i = 0; i < symbols->dim; i++) {
		Dsymbol *symbol = (Dsymbol *)symbols->data[i];
		char *kind = symbol->kind();

		// Skip these symbols.
		if (kind == "import" || kind == "static import" || 
			kind == "static assert" || kind == "alias" ||
			kind == "version" || kind == "debug")
		{
			continue;
		}
		// Flatten these symbols.
		else if (kind == "pragma" || kind == "attribute") {
			AggregateDeclaration *aggregate = (AggregateDeclaration *)symbol;
			DXPopulateItemWithSymbols(item, aggregate->members);
		}
		else if (kind == "static if") {
			ConditionalDeclaration *cond = (ConditionalDeclaration *)symbol;
			DXPopulateItemWithSymbols(item, cond->decl);
			DXPopulateItemWithSymbols(item, cond->elsedecl);
		}
		// Pass the rest normally, creating an item for each symbol.
		else {
			DXDSourceScannerItem *child = [[DXDSourceScannerItem alloc] initWithSymbol:symbol];
			[item addChild:child];
			[child release];
		}
	}
}


@implementation DXDSourceScannerItem

- (id)initWithSymbol:(Dsymbol *)s {
	symbol = s;
	char *kind = symbol->kind();
	unsigned int lineNum = symbol->loc.linnum;
	Identifier *ident = symbol->ident;
	
	NSString *dname;
	DXType dtype;
	NSRange dnameRange;
	NSRange dspanRange;
	
	dnameRange = NSMakeRange(symbol->loc.idstart, symbol->loc.idend - symbol->loc.idstart);
	if (symbol->loc.idstart > symbol->loc.idend)  dnameRange.length = 0; // safety
	
	dspanRange = NSMakeRange(symbol->loc.start,   symbol->loc.end - symbol->loc.start);
	if (symbol->loc.start > symbol->loc.end)  dspanRange.length = 0; // safety


	if (ident) {
		dname = [NSString stringWithUTF8String:ident->string];
	} else {
		//NSLog(@"Anonymous symbol of kind: %s", kind);
		dname = [NSString stringWithFormat:@"<anonymous %s>", kind];
	}
	
	if (kind == "function") {
		if ([dname isEqualToString:@"_staticCtor"]) {
			dname = @"static this()";
		}
		else if ([dname isEqualToString:@"_staticDtor"]) {
			dname = @"static ~this()";
		}
		else if ([dname isEqualToString:@"_dtor"]) {
			dname = @"~this()";
		}
		else if ([dname isEqualToString:@"__invariant"]) {
			dname = @"invariant";
		}
		else if ([dname hasPrefix:@"__unittest"]) {
			dname = @"unittest";
		}
		else {
			dname = [dname stringByAppendingString:@"()"];
		}
		bool hasBody = ((FuncDeclaration *)symbol)->fbody;
		dtype = hasBody ? DXTypeFunction : DXTypeFunctionDecl;
	}
	else if (kind == "constructor") {
		dname = @"this()";
		bool hasBody = ((FuncDeclaration *)symbol)->fbody;
		dtype = hasBody ? DXTypeFunction : DXTypeFunctionDecl;
	}
	else if (kind == "allocator") {
		dname = @"new()";
		bool hasBody = ((FuncDeclaration *)symbol)->fbody;
		dtype = hasBody ? DXTypeFunction : DXTypeFunctionDecl;
	}
	else if (kind == "deallocator") {
		dname = @"delete()";
		bool hasBody = ((FuncDeclaration *)symbol)->fbody;
		dtype = hasBody ? DXTypeFunction : DXTypeFunctionDecl;
	}
	else if (kind == "variable") {
		dtype = DXTypeRoot;
	}
	else if (kind == "mixin" || kind == "import") {
		dtype = DXTypeRoot;
	}
	else if (kind == "class" || kind == "struct" || kind == "union" || kind == "interface" || kind == "template") {
		dname = [NSString stringWithFormat:@"%s %@", kind, dname];
		dtype = DXTypeClass;
	}
	else if (kind == "anonymous class" || kind == "anonymous struct" || kind == "anonymous union") {
		dname = [NSString stringWithFormat:@"%s", kind];
		dtype = DXTypeRoot;
	}
	else if (kind == "enum") {
		dname = [NSString stringWithFormat:@"%s %@", kind, dname];
		dtype = ident ? DXTypeClass : DXTypeRoot; // Handle anonymous enum.
	}
	else if (kind == "enum member") {
		dtype = DXTypeRoot;
	}
	else if (kind == "typedef") {
		dtype = DXTypeTypedef;
	}
	else {
		//NSLog(@"Unsupported symbol kind: %s", kind);
		dname = [NSString stringWithFormat:@"<unknown: %s>", kind];
		dtype = DXTypeMark;
	}
	
	self = [super initWithName:dname type:dtype];
	[self setNameRange:dnameRange];
	[self setRange:dspanRange];
	
	if (kind == "class" || kind == "struct" || kind == "union" || kind == "interface" || kind == "template" || kind == "enum") {
		AggregateDeclaration *aggregate = (AggregateDeclaration *)symbol;
		DXPopulateItemWithSymbols(self, aggregate->members);
	}
	
	return self;
}

@end
