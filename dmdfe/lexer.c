
// Compiler implementation of the D programming language
// Copyright (c) 1999-2007 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

/* Lexical Analyzer */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <wchar.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#if IN_GCC || IN_DMDFE

#include <time.h>
#include "mem.h"

#else

#if __GNUC__
#include <time.h>
#endif

#if _WIN32
#include "..\root\mem.h"
#else
#include "../root/mem.h"
#endif
#endif

#include "stringtable.h"

#include "lexer.h"
#include "utf.h"
#include "identifier.h"
#include "id.h"
#include "module.h"

#if _WIN32 && __DMC__
// from \dm\src\include\setlocal.h
extern "C" char * __cdecl __locale_decpoint;
#endif

extern int HtmlNamedEntity(unsigned char *p, int length);

#define LS 0x2028	// UTF line separator
#define PS 0x2029	// UTF paragraph separator

/********************************************
 * Do our own char maps
 */

static unsigned char cmtable[256];

const int CMoctal =	0x1;
const int CMhex =	0x2;
const int CMidchar =	0x4;

inline unsigned char isoctal (unsigned char c) { return cmtable[c] & CMoctal; }
inline unsigned char ishex   (unsigned char c) { return cmtable[c] & CMhex; }
inline unsigned char isidchar(unsigned char c) { return cmtable[c] & CMidchar; }

static void cmtable_init()
{
    for (unsigned c = 0; c < sizeof(cmtable) / sizeof(cmtable[0]); c++)
    {
	if ('0' <= c && c <= '7')
	    cmtable[c] |= CMoctal;
	if (isdigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))
	    cmtable[c] |= CMhex;
	if (isalnum(c) || c == '_')
	    cmtable[c] |= CMidchar;
    }
}


/************************* Token **********************************************/

char *Token::tochars[TOKMAX];

void *Token::operator new(size_t size)
{   Token *t;

    if (Lexer::freelist)
    {
	t = Lexer::freelist;
	Lexer::freelist = t->next;
	return t;
    }

    return ::operator new(size);
}

#ifdef DEBUG
void Token::print()
{
    fprintf(stdmsg, "%s\n", toChars());
}
#endif

char *Token::toChars()
{   char *p;
    static char buffer[3 + 3 * sizeof(value) + 1];

    p = buffer;
    switch (value)
    {
	case TOKint32v:
#if IN_GCC
	    sprintf(buffer,"%d",(d_int32)int64value);
#else
	    sprintf(buffer,"%d",int32value);
#endif
	    break;

	case TOKuns32v:
	case TOKcharv:
	case TOKwcharv:
	case TOKdcharv:
#if IN_GCC
	    sprintf(buffer,"%uU",(d_uns32)uns64value);
#else
	    sprintf(buffer,"%uU",uns32value);
#endif
	    break;

	case TOKint64v:
	    sprintf(buffer,"%jdL",int64value);
	    break;

	case TOKuns64v:
	    sprintf(buffer,"%juUL",uns64value);
	    break;

#if IN_GCC
	case TOKfloat32v:
	case TOKfloat64v:
	case TOKfloat80v:
	    float80value.format(buffer, sizeof(buffer));
	    break;
	case TOKimaginary32v:
	case TOKimaginary64v:
	case TOKimaginary80v:
	    float80value.format(buffer, sizeof(buffer));
	    // %% buffer
	    strcat(buffer, "i");
	    break;
#else
	case TOKfloat32v:
	    sprintf(buffer,"%Lgf", float80value);
	    break;

	case TOKfloat64v:
	    sprintf(buffer,"%Lg", float80value);
	    break;

	case TOKfloat80v:
	    sprintf(buffer,"%LgL", float80value);
	    break;

	case TOKimaginary32v:
	    sprintf(buffer,"%Lgfi", float80value);
	    break;

	case TOKimaginary64v:
	    sprintf(buffer,"%Lgi", float80value);
	    break;

	case TOKimaginary80v:
	    sprintf(buffer,"%LgLi", float80value);
	    break;
#endif

	case TOKstring:
#if CSTRINGS
	    p = string;
#else
	{   OutBuffer buf;

	    buf.writeByte('"');
	    for (size_t i = 0; i < len; )
	    {	unsigned c;

		utf_decodeChar((unsigned char *)ustring, len, &i, &c);
		switch (c)
		{
		    case 0:
			break;

		    case '"':
		    case '\\':
			buf.writeByte('\\');
		    default:
			if (isprint(c))
			    buf.writeByte(c);
			else if (c <= 0x7F)
			    buf.printf("\\x%02x", c);
			else if (c <= 0xFFFF)
			    buf.printf("\\u%04x", c);
			else
			    buf.printf("\\U%08x", c);
			continue;
		}
		break;
	    }
	    buf.writeByte('"');
	    if (postfix)
		buf.writeByte('"');
	    buf.writeByte(0);
	    p = (char *)buf.extractData();
	}
#endif
	    break;

	case TOKidentifier:
	case TOKenum:
	case TOKstruct:
	case TOKimport:
	CASE_BASIC_TYPES:
	    p = ident->toChars();
	    break;

	default:
	    p = toChars(value);
	    break;
    }
    return p;
}

char *Token::toChars(enum TOK value)
{   char *p;
    static char buffer[3 + 3 * sizeof(value) + 1];

    p = tochars[value];
    if (!p)
    {	sprintf(buffer,"TOK%d",value);
	p = buffer;
    }
    return p;
}

/*************************** Lexer ********************************************/

Token *Lexer::freelist = NULL;
StringTable Lexer::stringtable;
OutBuffer Lexer::stringbuffer;

Lexer::Lexer(Module *mod,
	unsigned char *base, unsigned begoffset, unsigned endoffset,
	int doDocComment, int commentToken)
    : loc(mod, 1, 0, 0, 0, 0)
{
    //printf("Lexer::Lexer(%p,%d)\n",base,length);
    //printf("lexer.mod = %p, %p\n", mod, this->loc.mod);
    memset(&token,0,sizeof(token));
    this->base = base;
    this->end  = base + endoffset;
    p = base + begoffset;
	cc = 0;
    this->mod = mod;
    this->doDocComment = doDocComment;
    this->anyToken = 0;
    this->commentToken = commentToken;
    //initKeywords();

    /* If first line starts with '#!', ignore the line
     */

    if (p[0] == '#' && p[1] =='!')
    {
	p += 2; cc += 2;
	while (1)
	{   unsigned char c = *p;
	    switch (c)
	    {
		case '\n':
		    p++; cc++;
		    break;

		case '\r':
		    p++; cc++;
		    if (*p == '\n')
			p++; cc++;
		    break;

		case 0:
		case 0x1A:
		    break;

		default:
		    if (c & 0x80)
		    {   unsigned u = decodeUTF();
			if (u == PS || u == LS)
			    break;
		    }
		    p++; cc++;
		    continue;
	    }
	    break;
	}
	loc.linnum = 2;
    }
}


void Lexer::error(const char *format, ...)
{
    if (mod && !global.gag)
    {
	char *p = loc.toChars();
	if (*p)
	    fprintf(stdmsg, "%s: ", p);
	mem.free(p);

	va_list ap;
	va_start(ap, format);
	vfprintf(stdmsg, format, ap);
	va_end(ap);

	fprintf(stdmsg, "\n");
	fflush(stdmsg);

	if (global.errors >= 20)	// moderate blizzard of cascading messages
	    fatal();
    }
    global.errors++;
}

void Lexer::error(Loc loc, const char *format, ...)
{
    if (mod && !global.gag)
    {
	char *p = loc.toChars();
	if (*p)
	    fprintf(stdmsg, "%s: ", p);
	mem.free(p);

	va_list ap;
	va_start(ap, format);
	vfprintf(stdmsg, format, ap);
	va_end(ap);

	fprintf(stdmsg, "\n");
	fflush(stdmsg);

	if (global.errors >= 20)	// moderate blizzard of cascading messages
	    fatal();
    }
	
    global.errors++;
}

TOK Lexer::nextToken()
{   Token *t;

    if (token.next)
    {
	t = token.next;
	memcpy(&token,t,sizeof(Token));
	t->next = freelist;
	freelist = t;
    }
    else
    {
	scan(&token);
    }
    //token.print();
    return token.value;
}

Token *Lexer::peek(Token *ct)
{   Token *t;

    if (ct->next)
	t = ct->next;
    else
    {
	t = new Token();
	scan(t);
	t->next = NULL;
	ct->next = t;
    }
    return t;
}

/*********************************
 * tk is on the opening (.
 * Look ahead and return token that is past the closing ).
 */

Token *Lexer::peekPastParen(Token *tk)
{
    //printf("peekPastParen()\n");
    int parens = 1;
    int curlynest = 0;
    while (1)
    {
	tk = peek(tk);
	//tk->print();
	switch (tk->value)
	{
	    case TOKlparen:
		parens++;
		continue;

	    case TOKrparen:
		--parens;
		if (parens)
		    continue;
		tk = peek(tk);
		break;

	    case TOKlcurly:
		curlynest++;
		continue;

	    case TOKrcurly:
		if (--curlynest >= 0)
		    continue;
		break;

	    case TOKsemicolon:
		if (curlynest)
		    continue;
		break;

	    case TOKeof:
		break;

	    default:
		continue;
	}
	return tk;
    }
}

/**********************************
 * Determine if string is a valid Identifier.
 * Placed here because of commonality with Lexer functionality.
 * Returns:
 *	0	invalid
 */

int Lexer::isValidIdentifier(char *p)
{
    size_t len;
    size_t idx;

    if (!p || !*p)
	goto Linvalid;

    if (isdigit(*p))
	goto Linvalid;

    len = strlen(p);
    idx = 0;
    while (p[idx])
    {   dchar_t dc;

	char *q = utf_decodeChar((unsigned char *)p, len, &idx, &dc);
	if (q)
	    goto Linvalid;

	if (!((dc >= 0x80 && isUniAlpha(dc)) || isalnum(dc) || dc == '_'))
	    goto Linvalid;
    }
    return 1;

Linvalid:
    return 0;
}

/****************************
 * Turn next token in buffer into a token.
 */

void Lexer::scan(Token *t)
{
    unsigned lastLine = loc.linnum;
    unsigned linnum;

    t->blockComment = NULL;
    t->lineComment = NULL;
    while (1)
    {
	t->ptr = p;
	t->pos = cc;
	//printf("p = %p, *p = '%c'\n",p,*p);
	switch (*p)
	{
	    case 0:
	    case 0x1A:
		t->value = TOKeof;			// end of file
		return;

	    case ' ':
	    case '\t':
	    case '\v':
	    case '\f':
		p++; cc++;
		continue;			// skip white space

	    case '\r':
		p++; cc++;
		if (*p != '\n')			// if CR stands by itself
		    loc.linnum++;
		continue;			// skip white space

	    case '\n':
		p++; cc++;
		loc.linnum++;
		continue;			// skip white space

	    case '0':  	case '1':   case '2':   case '3':   case '4':
	    case '5':  	case '6':   case '7':   case '8':   case '9':
		t->value = number(t);
		return;

#if CSTRINGS
	    case '\'':
		t->value = charConstant(t, 0);
		return;

	    case '"':
		t->value = stringConstant(t,0);
		return;

	    case 'l':
	    case 'L':
		if (p[1] == '\'')
		{
		    p++; cc++;
		    t->value = charConstant(t, 1);
		    return;
		}
		else if (p[1] == '"')
		{
		    p++; cc++;
		    t->value = stringConstant(t, 1);
		    return;
		}
#else
	    case '\'':
		t->value = charConstant(t,0);
		return;

	    case 'r':
		if (p[1] != '"')
		    goto case_ident;
		p++; cc++;
	    case '`':
		t->value = wysiwygStringConstant(t, *p);
		return;

	    case 'x':
		if (p[1] != '"')
		    goto case_ident;
		p++; cc++;
		t->value = hexStringConstant(t);
		return;


	    case '"':
		t->value = escapeStringConstant(t,0);
		return;

	    case '\\':			// escaped string literal
	    {	unsigned c;

		stringbuffer.reset();
		do
		{
		    p++; cc++;
		    c = escapeSequence();
		    stringbuffer.writeUTF8(c);
		} while (*p == '\\');
		t->len = stringbuffer.offset;
		stringbuffer.writeByte(0);
		t->ustring = (unsigned char *)mem.malloc(stringbuffer.offset);
		memcpy(t->ustring, stringbuffer.data, stringbuffer.offset);
		t->postfix = 0;
		t->value = TOKstring;
		return;
	    }

	    case 'l':
	    case 'L':
#endif
	    case 'a':  	case 'b':   case 'c':   case 'd':   case 'e':
	    case 'f':  	case 'g':   case 'h':   case 'i':   case 'j':
	    case 'k':  	            case 'm':   case 'n':   case 'o':
	    case 'p':  	case 'q': /*case 'r':*/ case 's':   case 't':
	    case 'u':  	case 'v':   case 'w': /*case 'x':*/ case 'y':
	    case 'z':
	    case 'A':  	case 'B':   case 'C':   case 'D':   case 'E':
	    case 'F':  	case 'G':   case 'H':   case 'I':   case 'J':
	    case 'K':  	            case 'M':   case 'N':   case 'O':
	    case 'P':  	case 'Q':   case 'R':   case 'S':   case 'T':
	    case 'U':  	case 'V':   case 'W':   case 'X':   case 'Y':
	    case 'Z':
	    case '_':
	    case_ident:
	    {   unsigned char c;
		StringValue *sv;
		Identifier *id;

		do
		{
		    c = *++p; cc++;
		} while (isidchar(c) || (c & 0x80 && isUniAlpha(decodeUTF())));
		sv = stringtable.update((char *)t->ptr, p - t->ptr);
		id = (Identifier *) sv->ptrvalue;
		if (!id)
		{   id = new Identifier(sv->lstring.string,TOKidentifier);
		    sv->ptrvalue = id;
		}
		t->ident = id;
		t->value = (enum TOK) id->value;
		anyToken = 1;
		if (*t->ptr == '_')	// if special identifier token
		{
		    static char date[11+1];
		    static char time[8+1];
		    static char timestamp[24+1];

		    if (!date[0])	// lazy evaluation
		    {   time_t t;
			char *p;

			::time(&t);
			p = ctime(&t);
			assert(p);
			sprintf(date, "%.6s %.4s", p + 4, p + 20);
			sprintf(time, "%.8s", p + 11);
			sprintf(timestamp, "%.24s", p);
		    }

		    if (mod && id == Id::FILE)
		    {
			t->ustring = (unsigned char *)(loc.filename ? loc.filename : mod->ident->toChars());
			goto Lstring;
		    }
		    else if (mod && id == Id::LINE)
		    {
			t->value = TOKint64v;
			t->uns64value = loc.linnum;
		    }
		    else if (id == Id::DATE)
		    {
			t->ustring = (unsigned char *)date;
			goto Lstring;
		    }
		    else if (id == Id::TIME)
		    {
			t->ustring = (unsigned char *)time;
			goto Lstring;
		    }
		    else if (id == Id::VENDOR)
		    {
			t->ustring = (unsigned char *)"Digital Mars D";
			goto Lstring;
		    }
		    else if (id == Id::TIMESTAMP)
		    {
			t->ustring = (unsigned char *)timestamp;
		     Lstring:
			t->value = TOKstring;
		     Llen:
			t->postfix = 0;
			t->len = strlen((char *)t->ustring);
		    }
		    else if (id == Id::VERSIONX)
		    {	unsigned major = 0;
			unsigned minor = 0;

			for (char *p = global.version + 1; 1; p++)
			{
			    char c = *p;
			    if (isdigit(c))
				minor = minor * 10 + c - '0';
			    else if (c == '.')
			    {	major = minor;
				minor = 0;
			    }
			    else
				break;
			}
			t->value = TOKint64v;
			t->uns64value = major * 1000 + minor;
		    }
		}
		//printf("t->value = %d\n",t->value);
		return;
	    }

	    case '/':
		p++; cc++;
		switch (*p)
		{
		    case '=':
			p++; cc++;
			t->value = TOKdivass;
			return;

		    case '*':
			p++; cc++;
			linnum = loc.linnum;
			while (1)
			{
			    while (1)
			    {	unsigned char c = *p;
				switch (c)
				{
				    case '/':
					break;

				    case '\n':
					loc.linnum++;
					p++; cc++;
					continue;

				    case '\r':
					p++; cc++;
					if (*p != '\n')
					    loc.linnum++;
					continue;

				    case 0:
				    case 0x1A:
					error("unterminated /* */ comment");
					p = end; //cc = -1;
					t->value = TOKeof;
					return;

				    default:
					if (c & 0x80)
					{   unsigned u = decodeUTF();
					    if (u == PS || u == LS)
						loc.linnum++;
					}
					p++; cc++;
					continue;
				}
				break;
			    }
			    p++; cc++;
			    if (p[-2] == '*' && p - 3 != t->ptr)
				break;
			}
			if (commentToken)
			{
			    t->value = TOKcomment;
			    return;
			}
			else if (doDocComment && t->ptr[2] == '*' && p - 4 != t->ptr)
			{   // if /** but not /**/
			    getDocComment(t, lastLine == linnum);
			}
			continue;

		    case '/':		// do // style comments
			linnum = loc.linnum;
			while (1)
			{   unsigned char c = *++p; cc++;
			    switch (c)
			    {
				case '\n':
				    break;

				case '\r':
				    if (p[1] == '\n')
					p++; cc++;
				    break;

				case 0:
				case 0x1A:
				    if (commentToken)
				    {
					p = end; //cc = -1;
					t->value = TOKcomment;
					return;
				    }
				    if (doDocComment && t->ptr[2] == '/')
					getDocComment(t, lastLine == linnum);
				    p = end; //cc = -1;
				    t->value = TOKeof;
				    return;

				default:
				    if (c & 0x80)
				    {   unsigned u = decodeUTF();
					if (u == PS || u == LS)
					    break;
				    }
				    continue;
			    }
			    break;
			}

			if (commentToken)
			{
			    p++; cc++;
			    loc.linnum++;
			    t->value = TOKcomment;
			    return;
			}
			if (doDocComment && t->ptr[2] == '/')
			    getDocComment(t, lastLine == linnum);

			p++; cc++;
			loc.linnum++;
			continue;

		    case '+':
		    {	int nest;

			linnum = loc.linnum;
			p++; cc++;
			nest = 1;
			while (1)
			{   unsigned char c = *p;
			    switch (c)
			    {
				case '/':
				    p++; cc++;
				    if (*p == '+')
				    {
					p++; cc++;
					nest++;
				    }
				    continue;

				case '+':
				    p++; cc++;
				    if (*p == '/')
				    {
					p++; cc++;
					if (--nest == 0)
					    break;
				    }
				    continue;

				case '\r':
				    p++; cc++;
				    if (*p != '\n')
					loc.linnum++;
				    continue;

				case '\n':
				    loc.linnum++;
				    p++; cc++;
				    continue;

				case 0:
				case 0x1A:
				    error("unterminated /+ +/ comment");
				    p = end; //cc = -1;
				    t->value = TOKeof;
				    return;

				default:
				    if (c & 0x80)
				    {   unsigned u = decodeUTF();
					if (u == PS || u == LS)
					    loc.linnum++;
				    }
				    p++; cc++;
				    continue;
			    }
			    break;
			}
			if (commentToken)
			{
			    t->value = TOKcomment;
			    return;
			}
			if (doDocComment && t->ptr[2] == '+' && p - 4 != t->ptr)
			{   // if /++ but not /++/
			    getDocComment(t, lastLine == linnum);
			}
			continue;
		    }
		}
		t->value = TOKdiv;
		return;

	    case '.':
		p++; cc++;
		if (isdigit(*p))
		{   /* Note that we don't allow ._1 and ._ as being
		     * valid floating point numbers.
		     */
		    p--; cc--;
		    t->value = inreal(t);
		}
		else if (p[0] == '.')
		{
		    if (p[1] == '.')
		    {   p += 2; cc += 2;
			t->value = TOKdotdotdot;
		    }
		    else
		    {   p++; cc++;
			t->value = TOKslice;
		    }
		}
		else
		    t->value = TOKdot;
		return;

	    case '&':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKandass;
		}
		else if (*p == '&')
		{   p++; cc++;
		    t->value = TOKandand;
		}
		else
		    t->value = TOKand;
		return;

	    case '|':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKorass;
		}
		else if (*p == '|')
		{   p++; cc++;
		    t->value = TOKoror;
		}
		else
		    t->value = TOKor;
		return;

	    case '-':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKminass;
		}
#if 0
		else if (*p == '>')
		{   p++; cc++;
		    t->value = TOKarrow;
		}
#endif
		else if (*p == '-')
		{   p++; cc++;
		    t->value = TOKminusminus;
		}
		else
		    t->value = TOKmin;
		return;

	    case '+':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKaddass;
		}
		else if (*p == '+')
		{   p++; cc++;
		    t->value = TOKplusplus;
		}
		else
		    t->value = TOKadd;
		return;

	    case '<':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKle;			// <=
		}
		else if (*p == '<')
		{   p++; cc++;
		    if (*p == '=')
		    {   p++; cc++;
			t->value = TOKshlass;		// <<=
		    }
		    else
			t->value = TOKshl;		// <<
		}
		else if (*p == '>')
		{   p++; cc++;
		    if (*p == '=')
		    {   p++; cc++;
			t->value = TOKleg;		// <>=
		    }
		    else
			t->value = TOKlg;		// <>
		}
		else
		    t->value = TOKlt;			// <
		return;

	    case '>':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKge;			// >=
		}
		else if (*p == '>')
		{   p++; cc++;
		    if (*p == '=')
		    {   p++; cc++;
			t->value = TOKshrass;		// >>=
		    }
		    else if (*p == '>')
		    {	p++; cc++;
			if (*p == '=')
			{   p++; cc++;
			    t->value = TOKushrass;	// >>>=
			}
			else
			    t->value = TOKushr;		// >>>
		    }
		    else
			t->value = TOKshr;		// >>
		}
		else
		    t->value = TOKgt;			// >
		return;

	    case '!':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    if (*p == '=' && global.params.Dversion == 1)
		    {	p++; cc++;
			t->value = TOKnotidentity;	// !==
		    }
		    else
			t->value = TOKnotequal;		// !=
		}
		else if (*p == '<')
		{   p++; cc++;
		    if (*p == '>')
		    {	p++; cc++;
			if (*p == '=')
			{   p++; cc++;
			    t->value = TOKunord; // !<>=
			}
			else
			    t->value = TOKue;	// !<>
		    }
		    else if (*p == '=')
		    {	p++; cc++;
			t->value = TOKug;	// !<=
		    }
		    else
			t->value = TOKuge;	// !<
		}
		else if (*p == '>')
		{   p++; cc++;
		    if (*p == '=')
		    {	p++; cc++;
			t->value = TOKul;	// !>=
		    }
		    else
			t->value = TOKule;	// !>
		}
		else
		    t->value = TOKnot;		// !
		return;

	    case '=':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    if (*p == '=' && global.params.Dversion == 1)
		    {	p++; cc++;
			t->value = TOKidentity;		// ===
		    }
		    else
			t->value = TOKequal;		// ==
		}
		else
		    t->value = TOKassign;		// =
		return;

	    case '~':
		p++; cc++;
		if (*p == '=')
		{   p++; cc++;
		    t->value = TOKcatass;		// ~=
		}
		else
		    t->value = TOKtilde;		// ~
		return;

#define SINGLE(c,tok) case c: p++; cc++; t->value = tok; return;

	    SINGLE('(',	TOKlparen)
	    SINGLE(')', TOKrparen)
	    SINGLE('[', TOKlbracket)
	    SINGLE(']', TOKrbracket)
	    SINGLE('{', TOKlcurly)
	    SINGLE('}', TOKrcurly)
	    SINGLE('?', TOKquestion)
	    SINGLE(',', TOKcomma)
	    SINGLE(';', TOKsemicolon)
	    SINGLE(':', TOKcolon)
	    SINGLE('$', TOKdollar)

#undef SINGLE

#define DOUBLE(c1,tok1,c2,tok2)		\
	    case c1:			\
		p++; cc++;			\
		if (*p == c2)		\
		{   p++; cc++;		\
		    t->value = tok2;	\
		}			\
		else			\
		    t->value = tok1;	\
		return;

	    DOUBLE('*', TOKmul, '=', TOKmulass)
	    DOUBLE('%', TOKmod, '=', TOKmodass)
	    DOUBLE('^', TOKxor, '=', TOKxorass)

#undef DOUBLE

	    case '#':
		p++; cc++;
		pragma();
		continue;

	    default:
	    {	unsigned char c = *p;

		if (c & 0x80)
		{   unsigned u = decodeUTF();

		    // Check for start of unicode identifier
		    if (isUniAlpha(u))
			goto case_ident;

		    if (u == PS || u == LS)
		    {
			loc.linnum++;
			p++; cc++;
			continue;
		    }
		}
		if (isprint(c))
		    error("unsupported char '%c'", c);
		else
		    error("unsupported char 0x%02x", c);
		p++; cc++;
		continue;
	    }
	}
    }
}

/*******************************************
 * Parse escape sequence.
 */

unsigned Lexer::escapeSequence()
{   unsigned c;
    int n;
    int ndigits;

    c = *p;
    switch (c)
    {
	case '\'':
	case '"':
	case '?':
	case '\\':
	Lconsume:
		p++; cc++;
		break;

	case 'a':	c = 7;		goto Lconsume;
	case 'b':	c = 8;		goto Lconsume;
	case 'f':	c = 12;		goto Lconsume;
	case 'n':	c = 10;		goto Lconsume;
	case 'r':	c = 13;		goto Lconsume;
	case 't':	c = 9;		goto Lconsume;
	case 'v':	c = 11;		goto Lconsume;

	case 'u':
		ndigits = 4;
		goto Lhex;
	case 'U':
		ndigits = 8;
		goto Lhex;
	case 'x':
		ndigits = 2;
	Lhex:
		p++; cc++;
		c = *p;
		if (ishex(c))
		{   unsigned v;

		    n = 0;
		    v = 0;
		    while (1)
		    {
			if (isdigit(c))
			    c -= '0';
			else if (islower(c))
			    c -= 'a' - 10;
			else
			    c -= 'A' - 10;
			v = v * 16 + c;
			c = *++p; cc++;
			if (++n == ndigits)
			    break;
			if (!ishex(c))
			{   error("escape hex sequence has %d hex digits instead of %d", n, ndigits);
			    break;
			}
		    }
		    if (ndigits != 2 && !utf_isValidDchar(v))
			error("invalid UTF character \\U%08x", v);
		    c = v;
		}
		else
		    error("undefined escape hex sequence \\%c\n",c);
		break;

	case '&':			// named character entity
		cc++;
		for (unsigned char *idstart = ++p; 1; p++, cc++)
		{
		    switch (*p)
		    {
			case ';':
			    c = HtmlNamedEntity(idstart, p - idstart);
			    if (c == ~0)
			    {   error("unnamed character entity &%.*s;", (int)(p - idstart), idstart);
				c = ' ';
			    }
			    p++; cc++;
			    break;

			default:
			    if (isalpha(*p) ||
				(p != idstart + 1 && isdigit(*p)))
				continue;
			    error("unterminated named entity");
			    break;
		    }
		    break;
		}
		break;

	case 0:
	case 0x1A:			// end of file
		c = '\\';
		break;

	default:
		if (isoctal(c))
		{   unsigned char v;

		    n = 0;
		    v = 0;
		    do
		    {
			v = v * 8 + (c - '0');
			c = *++p; cc++;
		    } while (++n < 3 && isoctal(c));
		    c = v;
		}
		else
		    error("undefined escape sequence \\%c\n",c);
		break;
    }
    return c;
}

/**************************************
 */

TOK Lexer::wysiwygStringConstant(Token *t, int tc)
{   unsigned c;
    Loc start = loc;

    p++; cc++;
    stringbuffer.reset();
    while (1)
    {
	c = *p++; cc++;
	switch (c)
	{
	    case '\n':
		loc.linnum++;
		break;

	    case '\r':
		if (*p == '\n')
		    continue;	// ignore
		c = '\n';	// treat EndOfLine as \n character
		loc.linnum++;
		break;

	    case 0:
	    case 0x1A:
		error("unterminated string constant starting at %s", start.toChars());
		t->ustring = (unsigned char *)"";
		t->len = 0;
		t->postfix = 0;
		return TOKstring;

	    case '"':
	    case '`':
		if (c == tc)
		{
		    t->len = stringbuffer.offset;
		    stringbuffer.writeByte(0);
		    t->ustring = (unsigned char *)mem.malloc(stringbuffer.offset);
		    memcpy(t->ustring, stringbuffer.data, stringbuffer.offset);
		    stringPostfix(t);
		    return TOKstring;
		}
		break;

	    default:
		if (c & 0x80)
		{   p--; cc--;
		    unsigned u = decodeUTF();
		    p++; cc++;
		    if (u == PS || u == LS)
			loc.linnum++;
		    stringbuffer.writeUTF8(u);
		    continue;
		}
		break;
	}
	stringbuffer.writeByte(c);
    }
}

/**************************************
 * Lex hex strings:
 *	x"0A ae 34FE BD"
 */

TOK Lexer::hexStringConstant(Token *t)
{   unsigned c;
    Loc start = loc;
    unsigned n = 0;
    unsigned v;

    p++; cc++;
    stringbuffer.reset();
    while (1)
    {
	c = *p++; cc++;
	switch (c)
	{
	    case ' ':
	    case '\t':
	    case '\v':
	    case '\f':
		continue;			// skip white space

	    case '\r':
		if (*p == '\n')
		    continue;			// ignore
		// Treat isolated '\r' as if it were a '\n'
	    case '\n':
		loc.linnum++;
		continue;

	    case 0:
	    case 0x1A:
		error("unterminated string constant starting at %s", start.toChars());
		t->ustring = (unsigned char *)"";
		t->len = 0;
		t->postfix = 0;
		return TOKstring;

	    case '"':
		if (n & 1)
		{   error("odd number (%d) of hex characters in hex string", n);
		    stringbuffer.writeByte(v);
		}
		t->len = stringbuffer.offset;
		stringbuffer.writeByte(0);
		t->ustring = (unsigned char *)mem.malloc(stringbuffer.offset);
		memcpy(t->ustring, stringbuffer.data, stringbuffer.offset);
		stringPostfix(t);
		return TOKstring;

	    default:
		if (c >= '0' && c <= '9')
		    c -= '0';
		else if (c >= 'a' && c <= 'f')
		    c -= 'a' - 10;
		else if (c >= 'A' && c <= 'F')
		    c -= 'A' - 10;
		else if (c & 0x80)
		{   p--; cc--;
		    unsigned u = decodeUTF();
		    p++; cc++;
		    if (u == PS || u == LS)
			loc.linnum++;
		    else
			error("non-hex character \\u%x", u);
		}
		else
		    error("non-hex character '%c'", c);
		if (n & 1)
		{   v = (v << 4) | c;
		    stringbuffer.writeByte(v);
		}
		else
		    v = c;
		n++;
		break;
	}
    }
}

/**************************************
 */

TOK Lexer::escapeStringConstant(Token *t, int wide)
{   unsigned c;
    Loc start = loc;

    p++; cc++;
    stringbuffer.reset();
    while (1)
    {
	c = *p++; cc++;
	switch (c)
	{
	    case '\\':
		switch (*p)
		{
		    case 'u':
		    case 'U':
		    case '&':
			c = escapeSequence();
			stringbuffer.writeUTF8(c);
			continue;

		    default:
			c = escapeSequence();
			break;
		}
		break;

	    case '\n':
		loc.linnum++;
		break;

	    case '\r':
		if (*p == '\n')
		    continue;	// ignore
		c = '\n';	// treat EndOfLine as \n character
		loc.linnum++;
		break;

	    case '"':
		t->len = stringbuffer.offset;
		stringbuffer.writeByte(0);
		t->ustring = (unsigned char *)mem.malloc(stringbuffer.offset);
		memcpy(t->ustring, stringbuffer.data, stringbuffer.offset);
		stringPostfix(t);
		return TOKstring;

	    case 0:
	    case 0x1A:
		p--; cc++;
		error("unterminated string constant starting at %s", start.toChars());
		t->ustring = (unsigned char *)"";
		t->len = 0;
		t->postfix = 0;
		return TOKstring;

	    default:
		if (c & 0x80)
		{
		    p--; cc--;
		    c = decodeUTF();
		    if (c == LS || c == PS)
		    {	c = '\n';
			loc.linnum++;
		    }
		    p++; cc++;
		    stringbuffer.writeUTF8(c);
		    continue;
		}
		break;
	}
	stringbuffer.writeByte(c);
    }
}

/**************************************
 */

TOK Lexer::charConstant(Token *t, int wide)
{
    unsigned c;
    TOK tk = TOKcharv;

    //printf("Lexer::charConstant\n");
    p++; cc++;
    c = *p++; cc++;
    switch (c)
    {
	case '\\':
	    switch (*p)
	    {
		case 'u':
		    t->uns64value = escapeSequence();
		    tk = TOKwcharv;
		    break;

		case 'U':
		case '&':
		    t->uns64value = escapeSequence();
		    tk = TOKdcharv;
		    break;

		default:
		    t->uns64value = escapeSequence();
		    break;
	    }
	    break;

	case '\n':
	L1:
	    loc.linnum++;
	case '\r':
	case 0:
	case 0x1A:
	case '\'':
	    error("unterminated character constant");
	    return tk;

	default:
	    if (c & 0x80)
	    {
		p--; cc--;
		c = decodeUTF();
		p++; cc++;
		if (c == LS || c == PS)
		    goto L1;
		if (c < 0xD800 || (c >= 0xE000 && c < 0xFFFE))
		    tk = TOKwcharv;
		else
		    tk = TOKdcharv;
	    }
	    t->uns64value = c;
	    break;
    }

    if (*p != '\'')
    {	error("unterminated character constant");
	return tk;
    }
    p++; cc++;
    return tk;
}

/***************************************
 * Get postfix of string literal.
 */

void Lexer::stringPostfix(Token *t)
{
    switch (*p)
    {
	case 'c':
	case 'w':
	case 'd':
	    t->postfix = *p;
	    p++; cc++;
	    break;

	default:
	    t->postfix = 0;
	    break;
    }
}

/***************************************
 * Read \u or \U unicode sequence
 * Input:
 *	u	'u' or 'U'
 */

#if 0
unsigned Lexer::wchar(unsigned u)
{
    unsigned value;
    unsigned n;
    unsigned char c;
    unsigned nchars;

    nchars = (u == 'U') ? 8 : 4;
    value = 0;
    for (n = 0; 1; n++)
    {
	++p; cc++;
	if (n == nchars)
	    break;
	c = *p;
	if (!ishex(c))
	{   error("\\%c sequence must be followed by %d hex characters", u, nchars);
	    break;
	}
	if (isdigit(c))
	    c -= '0';
	else if (islower(c))
	    c -= 'a' - 10;
	else
	    c -= 'A' - 10;
	value <<= 4;
	value |= c;
    }
    return value;
}
#endif

/**************************************
 * Read in a number.
 * If it's an integer, store it in tok.TKutok.Vlong.
 *	integers can be decimal, octal or hex
 *	Handle the suffixes U, UL, LU, L, etc.
 * If it's double, store it in tok.TKutok.Vdouble.
 * Returns:
 *	TKnum
 *	TKdouble,...
 */

TOK Lexer::number(Token *t)
{
    // We use a state machine to collect numbers
    enum STATE { STATE_initial, STATE_0, STATE_decimal, STATE_octal, STATE_octale,
	STATE_hex, STATE_binary, STATE_hex0, STATE_binary0,
	STATE_hexh, STATE_error };
    enum STATE state;

    enum FLAGS
    {	FLAGS_decimal  = 1,		// decimal
	FLAGS_unsigned = 2,		// u or U suffix
	FLAGS_long     = 4,		// l or L suffix
    };
    enum FLAGS flags = FLAGS_decimal;

    int i;
    int base;
    unsigned c;
    unsigned char *start;
    TOK result;

    //printf("Lexer::number()\n");
    state = STATE_initial;
    base = 0;
    stringbuffer.reset();
    start = p;
	unsigned int cc_start = cc;
    while (1)
    {
	c = *p;
	switch (state)
	{
	    case STATE_initial:		// opening state
		if (c == '0')
		    state = STATE_0;
		else
		    state = STATE_decimal;
		break;

	    case STATE_0:
		flags = (FLAGS) (flags & ~FLAGS_decimal);
		switch (c)
		{
#if ZEROH
		    case 'H':			// 0h
		    case 'h':
			goto hexh;
#endif
		    case 'X':
		    case 'x':
			state = STATE_hex0;
			break;

		    case '.':
			if (p[1] == '.')	// .. is a separate token
			    goto done;
		    case 'i':
		    case 'f':
		    case 'F':
			goto real;
#if ZEROH
		    case 'E':
		    case 'e':
			goto case_hex;
#endif
		    case 'B':
		    case 'b':
			state = STATE_binary0;
			break;

		    case '0': case '1': case '2': case '3':
		    case '4': case '5': case '6': case '7':
			state = STATE_octal;
			break;

#if ZEROH
		    case '8': case '9': case 'A':
		    case 'C': case 'D': case 'F':
		    case 'a': case 'c': case 'd': case 'f':
		    case_hex:
			state = STATE_hexh;
			break;
#endif
		    case '_':
			state = STATE_octal;
			p++; cc++;
			continue;

		    case 'L':
			if (p[1] == 'i')
			    goto real;
			goto done;

		    default:
			goto done;
		}
		break;

	    case STATE_decimal:		// reading decimal number
		if (!isdigit(c))
		{
#if ZEROH
		    if (ishex(c)
			|| c == 'H' || c == 'h'
		       )
			goto hexh;
#endif
		    if (c == '_')		// ignore embedded _
		    {	p++; cc++;
			continue;
		    }
		    if (c == '.' && p[1] != '.')
			goto real;
		    else if (c == 'i' || c == 'f' || c == 'F' ||
			     c == 'e' || c == 'E')
		    {
	    real:	// It's a real number. Back up and rescan as a real
			p = start; cc = cc_start;
			return inreal(t);
		    }
		    else if (c == 'L' && p[1] == 'i')
			goto real;
		    goto done;
		}
		break;

	    case STATE_hex0:		// reading hex number
	    case STATE_hex:
		if (!ishex(c))
		{
		    if (c == '_')		// ignore embedded _
		    {	p++; cc++;
			continue;
		    }
		    if (c == '.' && p[1] != '.')
			goto real;
		    if (c == 'P' || c == 'p' || c == 'i')
			goto real;
		    if (state == STATE_hex0)
			error("Hex digit expected, not '%c'", c);
		    goto done;
		}
		state = STATE_hex;
		break;

#if ZEROH
	    hexh:
		state = STATE_hexh;
	    case STATE_hexh:		// parse numbers like 0FFh
		if (!ishex(c))
		{
		    if (c == 'H' || c == 'h')
		    {
			p++; cc++;
			base = 16;
			goto done;
		    }
		    else
		    {
			// Check for something like 1E3 or 0E24
			if (memchr((char *)stringbuffer.data, 'E', stringbuffer.offset) ||
			    memchr((char *)stringbuffer.data, 'e', stringbuffer.offset))
			    goto real;
			error("Hex digit expected, not '%c'", c);
			goto done;
		    }
		}
		break;
#endif

	    case STATE_octal:		// reading octal number
	    case STATE_octale:		// reading octal number with non-octal digits
		if (!isoctal(c))
		{
#if ZEROH
		    if (ishex(c)
			|| c == 'H' || c == 'h'
		       )
			goto hexh;
#endif
		    if (c == '_')		// ignore embedded _
		    {	p++; cc++;
			continue;
		    }
		    if (c == '.' && p[1] != '.')
			goto real;
		    if (c == 'i')
			goto real;
		    if (isdigit(c))
		    {
			state = STATE_octale;
		    }
		    else
			goto done;
		}
		break;

	    case STATE_binary0:		// starting binary number
	    case STATE_binary:		// reading binary number
		if (c != '0' && c != '1')
		{
#if ZEROH
		    if (ishex(c)
			|| c == 'H' || c == 'h'
		       )
			goto hexh;
#endif
		    if (c == '_')		// ignore embedded _
		    {	p++; cc++;
			continue;
		    }
		    if (state == STATE_binary0)
		    {	error("binary digit expected");
			state = STATE_error;
			break;
		    }
		    else
			goto done;
		}
		state = STATE_binary;
		break;

	    case STATE_error:		// for error recovery
		if (!isdigit(c))	// scan until non-digit
		    goto done;
		break;

	    default:
		assert(0);
	}
	stringbuffer.writeByte(c);
	p++; cc++;
    }
done:
    stringbuffer.writeByte(0);		// terminate string
    if (state == STATE_octale)
	error("Octal digit expected");

    uinteger_t n;			// unsigned >=64 bit integer type

    if (stringbuffer.offset == 2 && (state == STATE_decimal || state == STATE_0))
	n = stringbuffer.data[0] - '0';
    else
    {
	// Convert string to integer
#if __DMC__
	errno = 0;
	n = strtoull((char *)stringbuffer.data,NULL,base);
	if (errno == ERANGE)
	    error("integer overflow");
#else
	// Not everybody implements strtoull()
	char *p = (char *)stringbuffer.data;
	int r = 10, d;

	if (*p == '0')
	{
	    if (p[1] == 'x' || p[1] == 'X')
		p += 2, r = 16;
	    else if (p[1] == 'b' || p[1] == 'B')
		p += 2, r = 2;
	    else if (isdigit(p[1]))
		p += 1, r = 8;
	}

	n = 0;
	while (1)
	{
	    if (*p >= '0' && *p <= '9')
		d = *p - '0';
	    else if (*p >= 'a' && *p <= 'z')
		d = *p - 'a' + 10;
	    else if (*p >= 'A' && *p <= 'Z')
		d = *p - 'A' + 10;
	    else
		break;
	    if (d >= r)
		break;
	    if (n * r + d < n)
	    {
		error ("integer overflow");
		break;
	    }

	    n = n * r + d;
	    p++;
	}
#endif
	if (sizeof(n) > 8 &&
	    n > 0xFFFFFFFFFFFFFFFFULL)	// if n needs more than 64 bits
	    error("integer overflow");
    }

    // Parse trailing 'u', 'U', 'l' or 'L' in any combination
    while (1)
    {   unsigned char f;

	switch (*p)
	{   case 'U':
	    case 'u':
		f = FLAGS_unsigned;
		goto L1;

	    case 'l':
		if (1 || !global.params.useDeprecated)
		    error("'l' suffix is deprecated, use 'L' instead");
	    case 'L':
		f = FLAGS_long;
	    L1:
		p++; cc++;
		if (flags & f)
		    error("unrecognized token");
		flags = (FLAGS) (flags | f);
		continue;
	    default:
		break;
	}
	break;
    }

    switch (flags)
    {
	case 0:
	    /* Octal or Hexadecimal constant.
	     * First that fits: int, uint, long, ulong
	     */
	    if (n & 0x8000000000000000LL)
		    result = TOKuns64v;
	    else if (n & 0xFFFFFFFF00000000LL)
		    result = TOKint64v;
	    else if (n & 0x80000000)
		    result = TOKuns32v;
	    else
		    result = TOKint32v;
	    break;

	case FLAGS_decimal:
	    /* First that fits: int, long, long long
	     */
	    if (n & 0x8000000000000000LL)
	    {	    error("signed integer overflow");
		    result = TOKuns64v;
	    }
	    else if (n & 0xFFFFFFFF80000000LL)
		    result = TOKint64v;
	    else
		    result = TOKint32v;
	    break;

	case FLAGS_unsigned:
	case FLAGS_decimal | FLAGS_unsigned:
	    /* First that fits: uint, ulong
	     */
	    if (n & 0xFFFFFFFF00000000LL)
		    result = TOKuns64v;
	    else
		    result = TOKuns32v;
	    break;

	case FLAGS_decimal | FLAGS_long:
	    if (n & 0x8000000000000000LL)
	    {	    error("signed integer overflow");
		    result = TOKuns64v;
	    }
	    else
		    result = TOKint64v;
	    break;

	case FLAGS_long:
	    if (n & 0x8000000000000000LL)
		    result = TOKuns64v;
	    else
		    result = TOKint64v;
	    break;

	case FLAGS_unsigned | FLAGS_long:
	case FLAGS_decimal | FLAGS_unsigned | FLAGS_long:
	    result = TOKuns64v;
	    break;

	default:
	    #ifdef DEBUG
		printf("%x\n",flags);
	    #endif
	    assert(0);
    }
    t->uns64value = n;
    return result;
}

/**************************************
 * Read in characters, converting them to real.
 * Bugs:
 *	Exponent overflow not detected.
 *	Too much requested precision is not detected.
 */

TOK Lexer::inreal(Token *t)
#ifdef __DMC__
__in
{
    assert(*p == '.' || isdigit(*p));
}
__out (result)
{
    switch (result)
    {
	case TOKfloat32v:
	case TOKfloat64v:
	case TOKfloat80v:
	case TOKimaginary32v:
	case TOKimaginary64v:
	case TOKimaginary80v:
	    break;

	default:
	    assert(0);
    }
}
__body
#endif /* __DMC__ */
{   int dblstate;
    unsigned c;
    char hex;			// is this a hexadecimal-floating-constant?
    TOK result;

    //printf("Lexer::inreal()\n");
    stringbuffer.reset();
    dblstate = 0;
    hex = 0;
Lnext:
    while (1)
    {
	// Get next char from input
	c = *p++; cc++;
	//printf("dblstate = %d, c = '%c'\n", dblstate, c);
	while (1)
	{
	    switch (dblstate)
	    {
		case 0:			// opening state
		    if (c == '0')
			dblstate = 9;
		    else if (c == '.')
			dblstate = 3;
		    else
			dblstate = 1;
		    break;

		case 9:
		    dblstate = 1;
		    if (c == 'X' || c == 'x')
		    {	hex++;
			break;
		    }
		case 1:			// digits to left of .
		case 3:			// digits to right of .
		case 7:			// continuing exponent digits
		    if (!isdigit(c) && !(hex && isxdigit(c)))
		    {
			if (c == '_')
			    goto Lnext;	// ignore embedded '_'
			dblstate++;
			continue;
		    }
		    break;

		case 2:			// no more digits to left of .
		    if (c == '.')
		    {   dblstate++;
			break;
		    }
		case 4:			// no more digits to right of .
		    if ((c == 'E' || c == 'e') ||
			hex && (c == 'P' || c == 'p'))
		    {   dblstate = 5;
			hex = 0;	// exponent is always decimal
			break;
		    }
		    if (hex)
			error("binary-exponent-part required");
		    goto done;

		case 5:			// looking immediately to right of E
		    dblstate++;
		    if (c == '-' || c == '+')
			break;
		case 6:			// 1st exponent digit expected
		    if (!isdigit(c))
			error("exponent expected");
		    dblstate++;
		    break;

		case 8:			// past end of exponent digits
		    goto done;
	    }
	    break;
	}
	stringbuffer.writeByte(c);
    }
done:
    p--; cc--;

    stringbuffer.writeByte(0);

#if _WIN32 && __DMC__
    char *save = __locale_decpoint;
    __locale_decpoint = ".";
#endif
#ifdef IN_GCC
    t->float80value = real_t::parse((char *)stringbuffer.data, real_t::LongDouble);
#else
    t->float80value = strtold((char *)stringbuffer.data, NULL);
#endif
    errno = 0;
    switch (*p)
    {
	case 'F':
	case 'f':
#ifdef IN_GCC
	    real_t::parse((char *)stringbuffer.data, real_t::Float);
#else
	    strtof((char *)stringbuffer.data, NULL);
#endif
	    result = TOKfloat32v;
	    p++; cc++;
	    break;

	default:
#ifdef IN_GCC
	    real_t::parse((char *)stringbuffer.data, real_t::Double);
#else	    
	    strtod((char *)stringbuffer.data, NULL);
#endif
	    result = TOKfloat64v;
	    break;

	case 'l':
	    if (!global.params.useDeprecated)
		error("'l' suffix is deprecated, use 'L' instead");
	case 'L':
	    result = TOKfloat80v;
	    p++; cc++;
	    break;
    }
    if (*p == 'i' || *p == 'I')
    {
	if (!global.params.useDeprecated && *p == 'I')
	    error("'I' suffix is deprecated, use 'i' instead");
	p++; cc++;
	switch (result)
	{
	    case TOKfloat32v:
		result = TOKimaginary32v;
		break;
	    case TOKfloat64v:
		result = TOKimaginary64v;
		break;
	    case TOKfloat80v:
		result = TOKimaginary80v;
		break;
	}
    }
#if _WIN32 && __DMC__
    __locale_decpoint = save;
#endif
    if (errno == ERANGE)
	error("number is not representable");
    return result;
}

/*********************************************
 * Do pragma.
 * Currently, the only pragma supported is:
 *	#line linnum [filespec]
 */

void Lexer::pragma()
{
    Token tok;
    int linnum;
    char *filespec = NULL;
    Loc loc = this->loc;

    scan(&tok);
    if (tok.value != TOKidentifier || tok.ident != Id::line)
	goto Lerr;

    scan(&tok);
    if (tok.value == TOKint32v || tok.value == TOKint64v)
	linnum = tok.uns64value - 1;
    else
	goto Lerr;

    while (1)
    {
	switch (*p)
	{
	    case 0:
	    case 0x1A:
	    case '\n':
	    Lnewline:
		this->loc.linnum = linnum;
		if (filespec)
		    this->loc.filename = filespec;
		return;

	    case '\r':
		p++; cc++;
		if (*p != '\n')
		{   p--; cc--;
		    goto Lnewline;
		}
		continue;

	    case ' ':
	    case '\t':
	    case '\v':
	    case '\f':
		p++; cc++;
		continue;			// skip white space

	    case '_':
		if (mod && memcmp(p, "__FILE__", 8) == 0)
		{
		    p += 8; cc += 8;
		    filespec = mem.strdup(loc.filename ? loc.filename : mod->ident->toChars());
		}
		continue;

	    case '"':
		if (filespec)
		    goto Lerr;
		stringbuffer.reset();
		p++; cc++;
		while (1)
		{   unsigned c;

		    c = *p;
		    switch (c)
		    {
			case '\n':
			case '\r':
			case 0:
			case 0x1A:
			    goto Lerr;

			case '"':
			    stringbuffer.writeByte(0);
			    filespec = mem.strdup((char *)stringbuffer.data);
			    p++; cc++;
			    break;

			default:
			    if (c & 0x80)
			    {   unsigned u = decodeUTF();
				if (u == PS || u == LS)
				    goto Lerr;
			    }
			    stringbuffer.writeByte(c);
			    p++; cc++;
			    continue;
		    }
		    break;
		}
		continue;

	    default:
		if (*p & 0x80)
		{   unsigned u = decodeUTF();
		    if (u == PS || u == LS)
			goto Lnewline;
		}
		goto Lerr;
	}
    }

Lerr:
    error(loc, "#line integer [\"filespec\"]\\n expected");
}


/********************************************
 * Decode UTF character.
 * Issue error messages for invalid sequences.
 * Return decoded character, advance p to last character in UTF sequence.
 */

unsigned Lexer::decodeUTF()
{
    dchar_t u;
    unsigned char c;
    unsigned char *s = p;
    size_t len;
    size_t idx;
    char *msg;

    c = *s;
    assert(c & 0x80);

    // Check length of remaining string up to 6 UTF-8 characters
    for (len = 1; len < 6 && s[len]; len++)
	;

    idx = 0;
    msg = utf_decodeChar(s, len, &idx, &u);
    p += idx - 1;
    if (msg)
    {
	error("%s", msg);
    }
    return u;
}


/***************************************************
 * Parse doc comment embedded between t->ptr and p.
 * Remove trailing blanks and tabs from lines.
 * Replace all newlines with \n.
 * Remove leading comment character from each line.
 * Decide if it's a lineComment or a blockComment.
 * Append to previous one for this token.
 */

void Lexer::getDocComment(Token *t, unsigned lineComment)
{
    OutBuffer buf;
    unsigned char ct = t->ptr[2];
    unsigned char *q = t->ptr + 3;	// start of comment text
    int linestart = 0;

    unsigned char *qend = p;
    if (ct == '*' || ct == '+')
	qend -= 2;

    /* Scan over initial row of ****'s or ++++'s or ////'s
     */
    for (; q < qend; q++)
    {
	if (*q != ct)
	    break;
    }

    /* Remove trailing row of ****'s or ++++'s
     */
    if (ct != '/')
    {
	for (; q < qend; qend--)
	{
	    if (qend[-1] != ct)
		break;
	}
    }

    for (; q < qend; q++)
    {
	unsigned char c = *q;

	switch (c)
	{
	    case '*':
	    case '+':
		if (linestart && c == ct)
		{   linestart = 0;
		    /* Trim preceding whitespace up to preceding \n
		     */
		    while (buf.offset && (buf.data[buf.offset - 1] == ' ' || buf.data[buf.offset - 1] == '\t'))
			buf.offset--;
		    continue;
		}
		break;

	    case ' ':
	    case '\t':
		break;

	    case '\r':
		if (q[1] == '\n')
		    continue;		// skip the \r
		goto Lnewline;

	    default:
		if (c == 226)
		{
		    // If LS or PS
		    if (q[1] == 128 &&
			(q[2] == 168 || q[2] == 169))
		    {
			q += 2;
			goto Lnewline;
		    }
		}
		linestart = 0;
		break;

	    Lnewline:
		c = '\n';		// replace all newlines with \n
	    case '\n':
		linestart = 1;

		/* Trim trailing whitespace
		 */
		while (buf.offset && (buf.data[buf.offset - 1] == ' ' || buf.data[buf.offset - 1] == '\t'))
		    buf.offset--;

		break;
	}
	buf.writeByte(c);
    }

    // Always end with a newline
    if (!buf.offset || buf.data[buf.offset - 1] != '\n')
	buf.writeByte('\n');

    buf.writeByte(0);

    // It's a line comment if the start of the doc comment comes
    // after other non-whitespace on the same line.
    unsigned char** dc = (lineComment && anyToken)
			 ? &t->lineComment
			 : &t->blockComment;

    // Combine with previous doc comment, if any
    if (*dc)
	*dc = combineComments(*dc, (unsigned char *)buf.data);
    else
	*dc = (unsigned char *)buf.extractData();
}

/********************************************
 * Combine two document comments into one.
 */

unsigned char *Lexer::combineComments(unsigned char *c1, unsigned char *c2)
{
    unsigned char *c = c2;

    if (c1)
    {	c = c1;
	if (c2)
	{   size_t len1 = strlen((char *)c1);
	    size_t len2 = strlen((char *)c2);

	    c = (unsigned char *)mem.malloc(len1 + 1 + len2 + 1);
	    memcpy(c, c1, len1);
	    c[len1] = '\n';
	    memcpy(c + len1 + 1, c2, len2);
	    c[len1 + 1 + len2] = 0;
	}
    }
    return c;
}

/********************************************
 * Create an identifier in the string table.
 */

Identifier *Lexer::idPool(const char *s)
{   unsigned len;
    Identifier *id;
    StringValue *sv;

    len = strlen(s);
    sv = stringtable.update(s, len);
    id = (Identifier *) sv->ptrvalue;
    if (!id)
    {
	id = new Identifier(sv->lstring.string, TOKidentifier);
	sv->ptrvalue = id;
    }
    return id;
}

/****************************************
 */

struct Keyword
{   char *name;
    enum TOK value;
};

static Keyword keywords[] =
{
//    {	"",		TOK	},

    {	"this",		TOKthis		},
    {	"super",	TOKsuper	},
    {	"assert",	TOKassert	},
    {	"null",		TOKnull		},
    {	"true",		TOKtrue		},
    {	"false",	TOKfalse	},
    {	"cast",		TOKcast		},
    {	"new",		TOKnew		},
    {	"delete",	TOKdelete	},
    {	"throw",	TOKthrow	},
    {	"module",	TOKmodule	},
    {	"pragma",	TOKpragma	},
    {	"typeof",	TOKtypeof	},
    {	"typeid",	TOKtypeid	},

    {	"template",	TOKtemplate	},

    {	"void",		TOKvoid		},
    {	"byte",		TOKint8		},
    {	"ubyte",	TOKuns8		},
    {	"short",	TOKint16	},
    {	"ushort",	TOKuns16	},
    {	"int",		TOKint32	},
    {	"uint",		TOKuns32	},
    {	"long",		TOKint64	},
    {	"ulong",	TOKuns64	},
    {	"cent",		TOKcent,	},
    {	"ucent",	TOKucent,	},
    {	"float",	TOKfloat32	},
    {	"double",	TOKfloat64	},
    {	"real",		TOKfloat80	},

    {	"bool",		TOKbool		},
    {	"char",		TOKchar		},
    {	"wchar",	TOKwchar	},
    {	"dchar",	TOKdchar	},

    {	"ifloat",	TOKimaginary32	},
    {	"idouble",	TOKimaginary64	},
    {	"ireal",	TOKimaginary80	},

    {	"cfloat",	TOKcomplex32	},
    {	"cdouble",	TOKcomplex64	},
    {	"creal",	TOKcomplex80	},

    {	"delegate",	TOKdelegate	},
    {	"function",	TOKfunction	},

    {	"is",		TOKis		},
    {	"if",		TOKif		},
    {	"else",		TOKelse		},
    {	"while",	TOKwhile	},
    {	"for",		TOKfor		},
    {	"do",		TOKdo		},
    {	"switch",	TOKswitch	},
    {	"case",		TOKcase		},
    {	"default",	TOKdefault	},
    {	"break",	TOKbreak	},
    {	"continue",	TOKcontinue	},
    {	"synchronized",	TOKsynchronized	},
    {	"return",	TOKreturn	},
    {	"goto",		TOKgoto		},
    {	"try",		TOKtry		},
    {	"catch",	TOKcatch	},
    {	"finally",	TOKfinally	},
    {	"with",		TOKwith		},
    {	"asm",		TOKasm		},
    {	"foreach",	TOKforeach	},
    {	"foreach_reverse",	TOKforeach_reverse	},
    {	"scope",	TOKscope	},

    {	"struct",	TOKstruct	},
    {	"class",	TOKclass	},
    {	"interface",	TOKinterface	},
    {	"union",	TOKunion	},
    {	"enum",		TOKenum		},
    {	"import",	TOKimport	},
    {	"mixin",	TOKmixin	},
    {	"static",	TOKstatic	},
    {	"final",	TOKfinal	},
    {	"const",	TOKconst	},
    {	"typedef",	TOKtypedef	},
    {	"alias",	TOKalias	},
    {	"override",	TOKoverride	},
    {	"abstract",	TOKabstract	},
    {	"volatile",	TOKvolatile	},
    {	"debug",	TOKdebug	},
    {	"deprecated",	TOKdeprecated	},
    {	"in",		TOKin		},
    {	"out",		TOKout		},
    {	"inout",	TOKinout	},
    {	"lazy",		TOKlazy		},
    {	"auto",		TOKauto		},

    {	"align",	TOKalign	},
    {	"extern",	TOKextern	},
    {	"private",	TOKprivate	},
    {	"package",	TOKpackage	},
    {	"protected",	TOKprotected	},
    {	"public",	TOKpublic	},
    {	"export",	TOKexport	},

    {	"body",		TOKbody		},
    {	"invariant",	TOKinvariant	},
    {	"unittest",	TOKunittest	},
    {	"version",	TOKversion	},

    // Added after 1.0
    {	"ref",		TOKref		},
    {	"macro",	TOKmacro	},
};

int Token::isKeyword()
{
    for (unsigned u = 0; u < sizeof(keywords) / sizeof(keywords[0]); u++)
    {
	if (keywords[u].value == value)
	    return 1;
    }
    return 0;
}

void Lexer::initKeywords()
{   StringValue *sv;
    unsigned u;
    enum TOK v;
    unsigned nkeywords = sizeof(keywords) / sizeof(keywords[0]);

    if (global.params.Dversion == 1)
	nkeywords -= 2;

    cmtable_init();

    for (u = 0; u < nkeywords; u++)
    {	char *s;

	//printf("keyword[%d] = '%s'\n",u, keywords[u].name);
	s = keywords[u].name;
	v = keywords[u].value;
	sv = stringtable.insert(s, strlen(s));
	sv->ptrvalue = (void *) new Identifier(sv->lstring.string,v);

	//printf("tochars[%d] = '%s'\n",v, s);
	Token::tochars[v] = s;
    }

    Token::tochars[TOKeof]		= "EOF";
    Token::tochars[TOKlcurly]		= "{";
    Token::tochars[TOKrcurly]		= "}";
    Token::tochars[TOKlparen]		= "(";
    Token::tochars[TOKrparen]		= ")";
    Token::tochars[TOKlbracket]		= "[";
    Token::tochars[TOKrbracket]		= "]";
    Token::tochars[TOKsemicolon]	= ";";
    Token::tochars[TOKcolon]		= ":";
    Token::tochars[TOKcomma]		= ",";
    Token::tochars[TOKdot]		= ".";
    Token::tochars[TOKxor]		= "^";
    Token::tochars[TOKxorass]		= "^=";
    Token::tochars[TOKassign]		= "=";
    Token::tochars[TOKconstruct]	= "=";
    Token::tochars[TOKlt]		= "<";
    Token::tochars[TOKgt]		= ">";
    Token::tochars[TOKle]		= "<=";
    Token::tochars[TOKge]		= ">=";
    Token::tochars[TOKequal]		= "==";
    Token::tochars[TOKnotequal]		= "!=";
    Token::tochars[TOKnotidentity]	= "!is";
    Token::tochars[TOKtobool]		= "!!";

    Token::tochars[TOKunord]		= "!<>=";
    Token::tochars[TOKue]		= "!<>";
    Token::tochars[TOKlg]		= "<>";
    Token::tochars[TOKleg]		= "<>=";
    Token::tochars[TOKule]		= "!>";
    Token::tochars[TOKul]		= "!>=";
    Token::tochars[TOKuge]		= "!<";
    Token::tochars[TOKug]		= "!<=";

    Token::tochars[TOKnot]		= "!";
    Token::tochars[TOKtobool]		= "!!";
    Token::tochars[TOKshl]		= "<<";
    Token::tochars[TOKshr]		= ">>";
    Token::tochars[TOKushr]		= ">>>";
    Token::tochars[TOKadd]		= "+";
    Token::tochars[TOKmin]		= "-";
    Token::tochars[TOKmul]		= "*";
    Token::tochars[TOKdiv]		= "/";
    Token::tochars[TOKmod]		= "%";
    Token::tochars[TOKslice]		= "..";
    Token::tochars[TOKdotdotdot]	= "...";
    Token::tochars[TOKand]		= "&";
    Token::tochars[TOKandand]		= "&&";
    Token::tochars[TOKor]		= "|";
    Token::tochars[TOKoror]		= "||";
    Token::tochars[TOKarray]		= "[]";
    Token::tochars[TOKindex]		= "[i]";
    Token::tochars[TOKaddress]		= "&";
    Token::tochars[TOKstar]		= "*";
    Token::tochars[TOKtilde]		= "~";
    Token::tochars[TOKdollar]		= "$";
    Token::tochars[TOKcast]		= "cast";
    Token::tochars[TOKplusplus]		= "++";
    Token::tochars[TOKminusminus]	= "--";
    Token::tochars[TOKtype]		= "type";
    Token::tochars[TOKquestion]		= "?";
    Token::tochars[TOKneg]		= "-";
    Token::tochars[TOKuadd]		= "+";
    Token::tochars[TOKvar]		= "var";
    Token::tochars[TOKaddass]		= "+=";
    Token::tochars[TOKminass]		= "-=";
    Token::tochars[TOKmulass]		= "*=";
    Token::tochars[TOKdivass]		= "/=";
    Token::tochars[TOKmodass]		= "%=";
    Token::tochars[TOKshlass]		= "<<=";
    Token::tochars[TOKshrass]		= ">>=";
    Token::tochars[TOKushrass]		= ">>>=";
    Token::tochars[TOKandass]		= "&=";
    Token::tochars[TOKorass]		= "|=";
    Token::tochars[TOKcatass]		= "~=";
    Token::tochars[TOKcat]		= "~";
    Token::tochars[TOKcall]		= "call";
    Token::tochars[TOKidentity]		= "is";
    Token::tochars[TOKnotidentity]	= "!is";

    Token::tochars[TOKorass]		= "|=";
    Token::tochars[TOKidentifier]	= "identifier";

     // For debugging
    Token::tochars[TOKdotexp]		= "dotexp";
    Token::tochars[TOKdotti]		= "dotti";
    Token::tochars[TOKdotvar]		= "dotvar";
    Token::tochars[TOKdottype]		= "dottype";
    Token::tochars[TOKsymoff]		= "symoff";
    Token::tochars[TOKtypedot]		= "typedot";
    Token::tochars[TOKarraylength]	= "arraylength";
    Token::tochars[TOKarrayliteral]	= "arrayliteral";
    Token::tochars[TOKassocarrayliteral] = "assocarrayliteral";
    Token::tochars[TOKstructliteral]	= "structliteral";
    Token::tochars[TOKstring]		= "string";
    Token::tochars[TOKdsymbol]		= "symbol";
    Token::tochars[TOKtuple]		= "tuple";
    Token::tochars[TOKdeclaration]	= "declaration";
    Token::tochars[TOKdottd]		= "dottd";
}
