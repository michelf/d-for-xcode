/// Sample module to test parsing and syntax highlighting.
module mod;

import std.stdio;
import std.c.stdlib;
import selective : Object;
import aliased = module1;
import module2, module3;
import module4, aliased = module5, module6 : bindigns, aliased = module7 : bindigns;

// import not.an.import_;
/* import not.an.import_; */
/+ Test: /+ +/ import not.an.import_; +/

+1234_12349
-1234_56789
1.23_45
0x123
0b101

struct A
{

}

static ~this();

int var;
void functionDecl();
void functionDef()
body {
	byte[] a = import("file");
}

final class Class(A)
{
	this() { }
	~this() { }
	int member_var;
	void methodDecl(int a);
	void methodDef(A)(int a)
	{
		void test()
		in {
			void test();
		}
		out (a)
		{
		}
		body {
			test
		}
	}
	static void staticMethodDef() { }
	unittest
	{
	}
}
final struct Struct {
	int smember_var;
	static if (a) { void smethodDecl();}
	else {void testDecl();}
	void smethodDef() { }
	static void sstaticMethodDef() { }
}
unittest {

}
template Tmpl() {

}
enum Enum {
	A = 1,
}
typedef int TypeDef;
/**
 * Returns: test
 */
/++
 + Returns: test /++/
 +/
/// Test: Returns
/// Returns: Test

// Invalid characters

r"test"c
r"test"w
"@aabc@"

\n\r\t\xff

x"abcdef012345678"d

0x01234_123
0b010100101

q{ [test]{} } fd

q"@ABC"abc"ABC@";

q"ABC
abc ABC"
ABC";


klasdjf kjsdahf kasjldhfl asdf;asd ( {

klasdjf kjsdahf kasjldhfl asdf;asd char uint (http://a(test).)