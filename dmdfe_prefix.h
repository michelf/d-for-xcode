
// This prefix header applies to all files of the DMD front end and can be 
// used to define macros and other things.

// Compiling as a library: disable error messages and calls to exit().
#define DMD_LIB

// Symbol StringTable in DMD Front End clashes with symbol of the same name
// in a dev tool framework. This will rename the symbol in the front end.
#define StringTable DMD_StringTable