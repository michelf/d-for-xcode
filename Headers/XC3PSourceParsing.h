
// D for Xcode: Declarations of Xcode 3's source parsing classes
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

@class XCLanguageSpecification;

@interface XCSourceTokens : NSObject
{
    NSMutableSet *_tokens;
    BOOL _caseSensitive;
}

+ (int)_tokenForString:(id)fp8;
+ (int)addTokenForString:(id)fp8;
- (id)initWithArrayOfStrings:(id)fp8 caseSensitive:(BOOL)fp12;
- (void)dealloc;
- (void)addArrayOfStrings:(id)fp8;
- (void)setCaseSensitive:(BOOL)fp8;
- (BOOL)containsToken:(id)fp8;
- (int)tokenForString:(id)fp8;
- (id)allTokens;

@end

@interface XCSourceScanner : NSObject
{
    XCLanguageSpecification *_langSpec;
    NSMutableArray *_includedRules;
    XCSourceScanner *_lexer;
    int _langId;
    int _token;
    unsigned int _colorId:16;
    unsigned int _altColorId:16;
    unsigned int _startAtBOL:1;
    unsigned int _startAtColumnZero:1;
    unsigned int _isFoldable:1;
    unsigned int _ignoreToken:1;
    unsigned int _inheritsColor:1;
    unsigned int _altIgnoreToken:1;
    unsigned int _altInheritsColor:1;
    unsigned int _needToDirtyRightEdges:1;
}

- (id)initWithPropertyListDictionary:(id)fp8 language:(int)fp12;
- (id)initWithLanguageSpecification:(id)fp8;
- (void)dealloc;
- (id)description;
- (int)langId;
- (id)langSpec;
- (int)token;
- (id)lexer;
- (id)includedRules;
- (id)nodeToReuse:(id)fp8 forScanner:(id)fp12 inputStream:(id)fp16;
- (id)nodeForThisScanner:(id)fp8 atLocation:(unsigned int)fp12 inputStream:(id)fp16;
- (int)parseOneIncludedRule:(id)fp8 inTree:(id)fp12 withContext:(id)fp16 initialToken:(int)fp20 inputStream:(id)fp24 range:(struct _NSRange)fp28 dirtyRange:(struct _NSRange *)fp36 isLexing:(BOOL)fp40 reusedToken:(char *)fp44;
- (void)parseIncludedRules:(id)fp8 inTree:(id)fp12 withContext:(id)fp16 initialToken:(int)fp20 inputStream:(id)fp24 range:(struct _NSRange)fp28 dirtyRange:(struct _NSRange *)fp36;
- (BOOL)predictsRule:(int)fp8 inputStream:(id)fp12;
- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(id)fp20 range:(struct _NSRange)fp24 dirtyRange:(struct _NSRange *)fp32;
- (BOOL)canTokenize;
- (struct _NSRange)wordRangeInString:(id)fp8 fromIndex:(unsigned int)fp12 allowNonWords:(BOOL)fp16;
- (struct _NSRange)wordRangeInString:(id)fp8 fromIndex:(unsigned int)fp12;

@end


@interface XCRuleScanner : XCSourceScanner
{
    NSMutableArray *_rules;
    unsigned short _startChar;
}

- (id)initWithPropertyListDictionary:(id)fp8 language:(int)fp12;
- (void)dealloc;
- (BOOL)predictsRule:(int)fp8 inputStream:(id)fp12;
- (int)nextToken:(id)fp8 withItemArray:(id)fp12 inTree:(id)fp16 withContext:(id)fp20 initialToken:(int)fp24 range:(struct _NSRange)fp28 dirtyRange:(struct _NSRange *)fp36;
- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(id)fp20 range:(struct _NSRange)fp24 dirtyRange:(struct _NSRange *)fp32;

@end


@interface XCRegExScanner : XCSourceScanner
{
    NSCharacterSet *_startSet;
    NSCharacterSet *_invertedOtherSet;
    NSMutableArray *_regExes;
    NSMutableArray *_captureColors;
    struct _NSRange _previousTokenRange;
    BOOL _isSimpleToken;
    BOOL _altIsSimpleToken;
    BOOL _caseSensitive;
}

- (id)initWithPropertyListDictionary:(id)fp8 language:(int)fp12;
- (void)dealloc;
- (BOOL)predictsRule:(int)fp8 inputStream:(id)fp12;
- (int)tokenForString:(id)fp8 forRange:(struct _NSRange *)fp12 subItems:(id *)fp16;
- (int)nextToken:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 until:(unsigned int)fp20 subItems:(id *)fp24;
- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(id)fp20 range:(struct _NSRange)fp24 dirtyRange:(struct _NSRange *)fp32;
- (BOOL)canTokenize;
- (struct _NSRange)wordRangeInString:(id)fp8 fromIndex:(unsigned int)fp12;

@end


@interface XCKeywordScanner : XCSourceScanner
{
    NSCharacterSet *_startSet;
    NSCharacterSet *_invertedOtherSet;
    XCSourceTokens *_keywords;
    struct _NSRange _previousTokenRange;
    BOOL _caseSensitive;
    BOOL _isSimpleToken;
}

- (id)initWithPropertyListDictionary:(id)fp8 language:(int)fp12;
- (void)dealloc;
- (BOOL)predictsRule:(int)fp8 inputStream:(id)fp12;
- (int)nextToken:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16;
- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(id)fp20 range:(struct _NSRange)fp24 dirtyRange:(struct _NSRange *)fp32;
- (BOOL)canTokenize;
- (struct _NSRange)wordRangeInString:(id)fp8 fromIndex:(unsigned int)fp12;

@end


@interface XCPythonScanner : XCSourceScanner
{
}

- (int)_indentForLineWithRange:(struct _NSRange)fp8 inString:(id)fp16;
- (unsigned int)nextIndentForlocation:(unsigned int)fp8 inString:(id)fp12;
- (void)computeContext:(id)fp8 inString:(id)fp12 forLocation:(unsigned int)fp16;
- (int)parseOneIncludedRule:(id)fp8 inTree:(id)fp12 withContext:(id)fp16 initialToken:(int)fp20 inputStream:(id)fp24 range:(struct _NSRange)fp28 dirtyRange:(struct _NSRange *)fp36 isLexing:(BOOL)fp40 reusedToken:(char *)fp44;

@end


@interface XCBlockScanner : XCSourceScanner
{
    NSString *startString;
    NSString *endString;
    NSString *altEndString;
    unsigned short startChar;
    unsigned short endChar;
    unsigned short altEndChar;
    unsigned short escapeChar;
    unsigned int recursive:1;
    unsigned int dontIncludeEnd:1;
    unsigned int parseEndBeforeIncludedRules:1;
    unsigned int caseSensitive:1;
    int startToken;
    int endToken;
    int altEndToken;
    int altToken;
}

- (id)initWithPropertyListDictionary:(id)fp8 language:(int)fp12;
- (BOOL)predictsRule:(int)fp8 inputStream:(id)fp12;
- (int)nextToken:(id)fp8 withItem:(id)fp12 inTree:(id)fp16 withContext:(id)fp20 initialToken:(int)fp24 insideBlock:(BOOL)fp28 range:(struct _NSRange)fp32 dirtyRange:(struct _NSRange *)fp40 reusedToken:(char *)fp44;
- (int)actionForEndToken:(id)fp8 inContext:(id)fp12 inputStream:(id)fp16;
- (id)parseRecursive:(id)fp8 withContext:(id)fp12 inputStream:(id)fp16 range:(struct _NSRange)fp20 dirtyRange:(struct _NSRange *)fp28;
- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(id)fp20 range:(struct _NSRange)fp24 dirtyRange:(struct _NSRange *)fp32;

@end


@interface XCNumberScanner : XCRegExScanner
{
}

- (int)tokenForString:(id)fp8 forRange:(struct _NSRange *)fp12 subItems:(id *)fp16;

@end


@interface XCHTMLScanner : XCBlockScanner
{
}

- (int)actionForEndToken:(id)fp8 inContext:(id)fp12 inputStream:(id)fp16;
- (id)parseRecursive:(id)fp8 withContext:(id)fp12 inputStream:(id)fp16 range:(struct _NSRange)fp20 dirtyRange:(struct _NSRange *)fp28;

@end


@interface XCHTMLEntityScanner : XCBlockScanner
{
    NSDictionary *entityMap;
}

- (id)initWithPropertyListDictionary:(id)fp8 language:(int)fp12;
- (void)dealloc;
- (id)parse:(id)fp8 withContext:(id)fp12 initialToken:(int)fp16 inputStream:(id)fp20 range:(struct _NSRange)fp24 dirtyRange:(struct _NSRange *)fp32;

@end

