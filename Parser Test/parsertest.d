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

struct A {

}

static ~this();

int var;
void functionDecl();
void functionDef() { byte[] a = import("file"); }
final class Class {
	this() { }
	int member_var;
	void methodDecl();
	void methodDef() { }
	static void staticMethodDef() { }
}
final struct Struct {
	int smember_var;
	static if (a) { void smethodDecl();}
	else {void testDecl();}
	void smethodDef() { }
	static void sstaticMethodDef() { }
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
 + Returns: test
 +/
/// Test: Returns
/// Returns: Test

// Invalid characters

klasdjf kjsdahf kasjldhfl asdf;asd ( {

klasdjf kjsdahf kasjldhfl asdf;asd char uint (http://a(test).)