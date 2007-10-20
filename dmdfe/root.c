
// Copyright (c) 1999-2006 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#if _MSC_VER || ( __WIN32 && __GNUC__ )
#include <malloc.h>
#endif

#if _WIN32
#include <windows.h>
#include <direct.h>
#endif

#ifndef __WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#endif

#include "port.h"
#include "root.h"
#include "dchar.h"
#include "mem.h"

#if 0 //__SC__ //def DEBUG
extern "C" void __cdecl _assert(void *e, void *f, unsigned line)
{
    printf("Assert('%s','%s',%d)\n",e,f,line);
    fflush(stdout);
    *(char *)0 = 0;
}
#endif

/*************************************
 * Convert wchar string to ascii string.
 */

char *wchar2ascii(wchar_t *us)
{
    return wchar2ascii(us, wcslen(us));
}

char *wchar2ascii(wchar_t *us, unsigned len)
{
    unsigned i;
    char *p;

    p = (char *)mem.malloc(len + 1);
    for (i = 0; i <= len; i++)
	p[i] = (char) us[i];
    return p;
}

int wcharIsAscii(wchar_t *us)
{
    return wcharIsAscii(us, wcslen(us));
}

int wcharIsAscii(wchar_t *us, unsigned len)
{
    unsigned i;

    for (i = 0; i <= len; i++)
    {
	if (us[i] & ~0xFF)	// if high bits set
	    return 0;		// it's not ascii
    }
    return 1;
}


/***********************************
 * Compare length-prefixed strings (bstr).
 */

int bstrcmp(unsigned char *b1, unsigned char *b2)
{
    return (*b1 == *b2 && memcmp(b1 + 1, b2 + 1, *b2) == 0) ? 0 : 1;
}

/***************************************
 * Convert bstr into a malloc'd string.
 */

char *bstr2str(unsigned char *b)
{
    char *s;
    unsigned len;

    len = *b;
    s = (char *) mem.malloc(len + 1);
    s[len] = 0;
    return (char *)memcpy(s,b + 1,len);
}

/**************************************
 * Print error message and exit.
 */

void error(const char *format, ...)
{
#if !defined(DMD_LIB) || defined(LOG)
    va_list ap;
	
    va_start(ap, format);
    printf("Error: ");
    vprintf(format, ap);
    va_end( ap );
    printf("\n");
    fflush(stdout);
#endif
#ifndef DMD_LIB
    exit(EXIT_FAILURE);
#else
	throw __FILE__ " error()";
#endif
}

#if M_UNICODE
void error(const dchar *format, ...)
{
#if !defined(DMD_LIB) || defined(LOG)
    va_list ap;

    va_start(ap, format);
    printf("Error: ");
    vwprintf(format, ap);
    va_end( ap );
    printf("\n");
    fflush(stdout);
#endif
#ifndef DMD_LIB
    exit(EXIT_FAILURE);
#else
	thorw __FILE__ " error()";
#endif
}
#endif

void error_mem()
{
    error("out of memory");
}

/**************************************
 * Print warning message.
 */

void warning(const char *format, ...)
{
#if !defined(DMD_LIB) || defined(LOG)
    va_list ap;

    va_start(ap, format);
    printf("Warning: ");
    vprintf(format, ap);
    va_end( ap );
    printf("\n");
    fflush(stdout);
#endif
}

/****************************** Object ********************************/

int Object::equals(Object *o)
{
    return o == this;
}

hash_t Object::hashCode()
{
    return (hash_t) this;
}

int Object::compare(Object *obj)
{
    return this - obj;
}

void Object::print()
{
    printf("%s %p\n", toChars(), this);
}

char *Object::toChars()
{
    return "Object";
}

dchar *Object::toDchars()
{
#if M_UNICODE
    return L"Object";
#else
    return toChars();
#endif
}

int Object::dyncast()
{
    return 0;
}

void Object::toBuffer(OutBuffer *b)
{
    b->writestring("Object");
}

void Object::mark()
{
}

/****************************** String ********************************/

String::String(char *str, int ref)
{
    this->str = ref ? str : mem.strdup(str);
    this->ref = ref;
}

String::~String()
{
    mem.free(str);
}

void String::mark()
{
    mem.mark(str);
}

hash_t String::calcHash(const char *str, size_t len)
{
    hash_t hash = 0;

    for (;;)
    {
	switch (len)
	{
	    case 0:
		return hash;

	    case 1:
		hash *= 37;
		hash += *(uint8_t *)str;
		return hash;

	    case 2:
		hash *= 37;
		hash += *(uint16_t *)str;
		return hash;

	    case 3:
		hash *= 37;
		hash += (*(uint16_t *)str << 8) +
			((uint8_t *)str)[2];
		return hash;

	    default:
		hash *= 37;
		hash += *(uint32_t *)str;
		str += 4;
		len -= 4;
		break;
	}
    }
}

hash_t String::calcHash(const char *str)
{
    return calcHash(str, strlen(str));
}

hash_t String::hashCode()
{
    return calcHash(str, strlen(str));
}

unsigned String::len()
{
    return strlen(str);
}

int String::equals(Object *obj)
{
    return strcmp(str,((String *)obj)->str) == 0;
}

int String::compare(Object *obj)
{
    return strcmp(str,((String *)obj)->str);
}

char *String::toChars()
{
    return str;
}

void String::print()
{
    printf("String '%s'\n",str);
}


/****************************** FileName ********************************/

FileName::FileName(char *str, int ref)
    : String(str,ref)
{
}

char *FileName::combine(char *path, char *name)
{   char *f;
    size_t pathlen;
    size_t namelen;

    if (!path || !*path)
	return name;
    pathlen = strlen(path);
    namelen = strlen(name);
    f = (char *)mem.malloc(pathlen + 1 + namelen + 1);
    memcpy(f, path, pathlen);
#if _WIN32
    if (path[pathlen - 1] != '\\' && path[pathlen - 1] != ':')
    {	f[pathlen] = '\\';
	pathlen++;
    }
#else
    if (path[pathlen - 1] != '/')
    {	f[pathlen] = '/';
	pathlen++;
    }
#endif
    memcpy(f + pathlen, name, namelen + 1);
    return f;
}

FileName::FileName(char *path, char *name)
    : String(combine(path,name),1)
{
}

// Split a path into an Array of paths
Array *FileName::splitPath(const char *path)
{
    char c = 0;				// unnecessary initializer is for VC /W4
    const char *p;
    OutBuffer buf;
    Array *array;

    array = new Array();
    if (path)
    {
	p = path;
	do
	{   char instring = 0;

	    while (isspace(*p))		// skip leading whitespace
		p++;
	    buf.reserve(strlen(p) + 1);	// guess size of path
	    for (; ; p++)
	    {
		c = *p;
		switch (c)
		{
		    case '"':
			instring ^= 1;	// toggle inside/outside of string
			continue;

#if MACINTOSH
		    case ',':
#endif
#if _WIN32
		    case ';':
#else
		    case ':':
#endif
			p++;
			break;		// note that ; cannot appear as part
					// of a path, quotes won't protect it

		    case 0x1A:		// ^Z means end of file
		    case 0:
			break;

		    case '\r':
			continue;	// ignore carriage returns

#ifndef __WIN32
		    case '~':
			buf.writestring(getenv("HOME"));
			continue;
#endif

		    case ' ':
		    case '\t':		// tabs in filenames?
			if (!instring)	// if not in string
			    break;	// treat as end of path
		    default:
			buf.writeByte(c);
			continue;
		}
		break;
	    }
	    if (buf.offset)		// if path is not empty
	    {
		buf.writeByte(0);	// to asciiz
		array->push(buf.extractData());
	    }
	} while (c);
    }
    return array;
}

hash_t FileName::hashCode()
{
#if _WIN32
    // We need a different hashCode because it must be case-insensitive
    size_t len = strlen(str);
    hash_t hash = 0;
    unsigned char *s = (unsigned char *)str;

    for (;;)
    {
	switch (len)
	{
	    case 0:
		return hash;

	    case 1:
		hash *= 37;
		hash += *(uint8_t *)s | 0x20;
		return hash;

	    case 2:
		hash *= 37;
		hash += *(uint16_t *)s | 0x2020;
		return hash;

	    case 3:
		hash *= 37;
		hash += ((*(uint16_t *)s << 8) +
			 ((uint8_t *)s)[2]) | 0x202020;
		break;

	    default:
		hash *= 37;
		hash += *(uint32_t *)s | 0x20202020;
		s += 4;
		len -= 4;
		break;
	}
    }
#else
    // darwin HFS is case insensitive, though...
    return String::hashCode();
#endif
}

int FileName::compare(Object *obj)
{
#if _WIN32
    return stricmp(str,((FileName *)obj)->str);
#else
    return String::compare(obj);
#endif
}

int FileName::equals(Object *obj)
{
#if _WIN32
    return stricmp(str,((FileName *)obj)->str) == 0;
#else
    return String::equals(obj);
#endif
}

/************************************
 * Return !=0 if absolute path name.
 */

int FileName::absolute(const char *name)
{
#if _WIN32
    return (*name == '\\') ||
	   (*name == '/')  ||
	   (*name && name[1] == ':');
#else
    return (*name == '/');
#endif
}

/********************************
 * Return filename extension (read-only).
 * If there isn't one, return NULL.
 */

char *FileName::ext(const char *str)
{
    char *e;
    size_t len = strlen(str);

    e = (char *)str + len;
    for (;;)
    {
	switch (*e)
	{   case '.':
		return e + 1;
#if _WIN32
	    case '\\':
	    case ':':
		break;
#else
	    case '/':
	        break;
#endif
	    default:
		if (e == str)
		    break;
		e--;
		continue;
	}
	return NULL;
    }
}

char *FileName::ext()
{
    return ext(str);
}

/********************************
 * Return filename name excluding path (read-only).
 */

char *FileName::name(const char *str)
{
    char *e;
    size_t len = strlen(str);

    e = (char *)str + len;
    for (;;)
    {
	switch (*e)
	{
#if _WIN32
	    case '\\':
	    case ':':
		return e + 1;
#else
	    case '/':
                return e + 1;
#endif
	    default:
		if (e == str)
		    break;
		e--;
		continue;
	}
	return e;
    }
}

char *FileName::name()
{
    return name(str);
}

/**************************************
 * Return path portion of str.
 * Path will does not include trailing path separator.
 */

char *FileName::path(const char *str)
{
    char *n = name(str);
    char *path;
    size_t pathlen;

    if (n > str)
    {
#if _WIN32
	if (n[-1] == '\\')
	    n--;
#else
	if (n[-1] == '/')
	    n--;
#endif
    }
    pathlen = n - str;
    path = (char *)mem.malloc(pathlen + 1);
    memcpy(path, str, pathlen);
    path[pathlen] = 0;
    return path;
}

/**************************************
 * Replace filename portion of path.
 */

char *FileName::replaceName(char *path, char *name)
{   char *f;
    char *n;
    size_t pathlen;
    size_t namelen;

    if (absolute(name))
	return name;

    n = FileName::name(path);
    if (n == path)
	return name;
    pathlen = n - path;
    namelen = strlen(name);
    f = (char *)mem.malloc(pathlen + 1 + namelen + 1);
    memcpy(f, path, pathlen);
#if _WIN32
    if (path[pathlen - 1] != '\\' && path[pathlen - 1] != ':')
    {	f[pathlen] = '\\';
	pathlen++;
    }
#else
    if (path[pathlen - 1] != '/')
    {	f[pathlen] = '/';
	pathlen++;
    }
#endif
    memcpy(f + pathlen, name, namelen + 1);
    return f;
}

/***************************
 */

FileName *FileName::defaultExt(const char *name, const char *ext)
{
    char *e;
    char *s;
    size_t len;
    size_t extlen;

    e = FileName::ext(name);
    if (e)				// if already has an extension
	return new FileName((char *)name, 0);

    len = strlen(name);
    extlen = strlen(ext);
    s = (char *)alloca(len + 1 + extlen + 1);
    memcpy(s,name,len);
    s[len] = '.';
    memcpy(s + len + 1, ext, extlen + 1);
    return new FileName(s, 0);
}

/***************************
 */

FileName *FileName::forceExt(const char *name, const char *ext)
{
    char *e;
    char *s;
    size_t len;
    size_t extlen;

    e = FileName::ext(name);
    if (e)				// if already has an extension
    {
	len = e - name;
	extlen = strlen(ext);

	s = (char *)alloca(len + extlen + 1);
	memcpy(s,name,len);
	memcpy(s + len, ext, extlen + 1);
	return new FileName(s, 0);
    }
    else
	return defaultExt(name, ext);	// doesn't have one
}

/******************************
 * Return !=0 if extensions match.
 */

int FileName::equalsExt(const char *ext)
{   const char *e;

    e = FileName::ext();
    if (!e && !ext)
	return 1;
    if (!e || !ext)
	return 0;
#if _WIN32
    return stricmp(e,ext) == 0;
#else
    return strcmp(e,ext) == 0;
#endif
}

/*************************************
 * Copy file from this to to.
 */

void FileName::CopyTo(FileName *to)
{
    File file(this);

#if _WIN32
    file.touchtime = mem.malloc(sizeof(WIN32_FIND_DATAA));	// keep same file time
#else
    file.touchtime = mem.malloc(sizeof(struct stat)); // keep same file time
#endif
    file.readv();
    file.name = to;
    file.writev();
}

/*************************************
 * Search Path for file.
 * Input:
 *	cwd	if !=0, search current directory before searching path
 */

char *FileName::searchPath(Array *path, char *name, int cwd)
{
    if (absolute(name))
    {
	return exists(name) ? name : NULL;
    }
    if (cwd)
    {
	if (exists(name))
	    return name;
    }
    if (path)
    {	unsigned i;

	for (i = 0; i < path->dim; i++)
	{
	    char *p = (char *)path->data[i];
	    char *n = combine(p, name);

	    if (exists(n))
		return n;
	}
    }
    return NULL;
}

int FileName::exists(const char *name)
{
#if _WIN32
    DWORD dw;
    int result;

    dw = GetFileAttributesA(name);
    if (dw == -1L)
	result = 0;
    else if (dw & FILE_ATTRIBUTE_DIRECTORY)
	result = 2;
    else
	result = 1;
    return result;
#else
    struct stat st;

    if (stat(name, &st) < 0)
	return 0;
    if (S_ISDIR(st.st_mode))
	return 2;
    return 1;
#endif
}

void FileName::ensurePathExists(const char *path)
{
    //printf("FileName::ensurePathExists(%s)\n", path ? path : "");
    if (path && *path)
    {
	if (!exists(path))
	{
	    char *p = FileName::path(path);
	    if (*p)
	    {
#if _WIN32
		size_t len = strlen(p);
		if (len > 2 && p[-1] == ':')
		{   mem.free(p);
		    return;
		}
#endif
		ensurePathExists(p);
		mem.free(p);
	    }
#if _WIN32
	    if (path[strlen(path) - 1] != '\\')
#else
	    if (path[strlen(path) - 1] != '\\')
#endif
	    {
		//printf("mkdir(%s)\n", path);
#if _WIN32
		if (mkdir(path))
#else
		if (mkdir(path, 0777))
#endif
		    error("cannot create directory %s", path);
	    }
	}
    }
}

/****************************** File ********************************/

File::File(FileName *n)
{
    ref = 0;
    buffer = NULL;
    len = 0;
    touchtime = NULL;
    name = n;
}

File::File(char *n)
{
    ref = 0;
    buffer = NULL;
    len = 0;
    touchtime = NULL;
    name = new FileName(n, 0);
}

File::~File()
{
    if (buffer)
    {
	if (ref == 0)
	    mem.free(buffer);
#if _WIN32
	else if (ref == 2)
	    UnmapViewOfFile(buffer);
#endif
    }
    if (touchtime)
	mem.free(touchtime);
}

void File::mark()
{
    mem.mark(buffer);
    mem.mark(touchtime);
    mem.mark(name);
}

/*************************************
 */

int File::read()
{
#if _WIN32
    DWORD size;
    DWORD numread;
    HANDLE h;
    int result = 0;
    char *name;

    name = this->name->toChars();
    h = CreateFileA(name,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,0);
    if (h == INVALID_HANDLE_VALUE)
	goto err1;

    if (!ref)
	mem.free(buffer);
    ref = 0;

    size = GetFileSize(h,NULL);
    buffer = (unsigned char *) mem.malloc(size + 2);
    if (!buffer)
	goto err2;

    if (ReadFile(h,buffer,size,&numread,NULL) != TRUE)
	goto err2;

    if (numread != size)
	goto err2;

    if (touchtime)
    {
	if (!GetFileTime(h, NULL, NULL, &((WIN32_FIND_DATAA *)touchtime)->ftLastWriteTime))
	    goto err2;
    }

    if (!CloseHandle(h))
	goto err;

    len = size;

    // Always store a wchar ^Z past end of buffer so scanner has a sentinel
    buffer[size] = 0;		// ^Z is obsolete, use 0
    buffer[size + 1] = 0;
    return 0;

err2:
    CloseHandle(h);
err:
    mem.free(buffer);
    buffer = NULL;
    len = 0;

err1:
    result = 1;
    return result;
#else
    off_t size;
    ssize_t numread;
    int fd;
    struct stat buf;
    int result = 0;
    char *name;

    name = this->name->toChars();
    //printf("File::read('%s')\n",name);
    fd = open(name, O_RDONLY);
    if (fd == -1)
    {	result = errno;
	//printf("\topen error, errno = %d\n",errno);
	goto err1;
    }

    if (!ref)
	mem.free(buffer);
    ref = 0;       // we own the buffer now

    //printf("\tfile opened\n");
    if (fstat(fd, &buf))
    {
	printf("\tfstat error, errno = %d\n",errno);
        goto err2;
    }
    size = buf.st_size;
    buffer = (unsigned char *) mem.malloc(size + 2);
    if (!buffer)
    {
	printf("\tmalloc error, errno = %d\n",errno);
	goto err2;
    }

    numread = ::read(fd, buffer, size);
    if (numread != size)
    {
	printf("\tread error, errno = %d\n",errno);
	goto err2;
    }

    if (touchtime)
        memcpy(touchtime, &buf, sizeof(buf));

    if (close(fd) == -1)
    {
	printf("\tclose error, errno = %d\n",errno);
	goto err;
    }

    len = size;

    // Always store a wchar ^Z past end of buffer so scanner has a sentinel
    buffer[size] = 0;		// ^Z is obsolete, use 0
    buffer[size + 1] = 0;
    return 0;

    err2:
    close(fd);
    err:
    mem.free(buffer);
    buffer = NULL;
    len = 0;

    err1:
    result = 1;
    return result;
#endif
}

/*****************************
 * Read a file with memory mapped file I/O.
 */

int File::mmread()
{
#if _WIN32
    HANDLE hFile;
    HANDLE hFileMap;
    DWORD size;
    char *name;

    name = this->name->toChars();
    hFile = CreateFile(name, GENERIC_READ,
			FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
	goto Lerr;
    size = GetFileSize(hFile, NULL);
    //printf(" file created, size %d\n", size);

    hFileMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,0,size,NULL);
    if (CloseHandle(hFile) != TRUE)
	goto Lerr;

    if (hFileMap == NULL)
	goto Lerr;

    //printf(" mapping created\n");

    if (!ref)
	mem.free(buffer);
    ref = 2;
    buffer = (unsigned char *)MapViewOfFileEx(hFileMap, FILE_MAP_READ,0,0,size,NULL);
    if (CloseHandle(hFileMap) != TRUE)
	goto Lerr;
    if (buffer == NULL)			// mapping view failed
	goto Lerr;

    len = size;
    //printf(" buffer = %p\n", buffer);

    return 0;

Lerr:
    return GetLastError();			// failure
#else
    return read();
#endif
}

/*********************************************
 * Write a file.
 * Returns:
 *	0	success
 */

int File::write()
{
#if _WIN32
    HANDLE h;
    DWORD numwritten;
    char *name;

    name = this->name->toChars();
    h = CreateFileA(name,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (h == INVALID_HANDLE_VALUE)
	goto err;

    if (WriteFile(h,buffer,len,&numwritten,NULL) != TRUE)
	goto err2;

    if (len != numwritten)
	goto err2;
    
    if (touchtime) {
        SetFileTime(h, NULL, NULL, &((WIN32_FIND_DATAA *)touchtime)->ftLastWriteTime);
    }
    if (!CloseHandle(h))
	goto err;
    return 0;

err2:
    CloseHandle(h);
    DeleteFileA(name);
err:
    return 1;
#else
    int fd;
    ssize_t numwritten;
    char *name;

    name = this->name->toChars();
    fd = open(name, O_CREAT | O_WRONLY | O_TRUNC, 0660);
    if (fd == -1)
	goto err;

    numwritten = ::write(fd, buffer, len);
    if (len != numwritten)
	goto err2;
    
    if (close(fd) == -1)
	goto err;

    if (touchtime)
    {   struct utimbuf ubuf;

        ubuf.actime = ((struct stat *)touchtime)->st_atime;
        ubuf.modtime = ((struct stat *)touchtime)->st_mtime;
	if (utime(name, &ubuf))
	    goto err;
    }
    return 0;

    err2:
    close(fd);
    ::remove(name);
    err:
    return 1;
#endif
}

/*********************************************
 * Append to a file.
 * Returns:
 *	0	success
 */

int File::append()
{
#if _WIN32
    HANDLE h;
    DWORD numwritten;
    char *name;

    name = this->name->toChars();
    h = CreateFileA(name,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,
	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if (h == INVALID_HANDLE_VALUE)
	goto err;

#if 1
    SetFilePointer(h, 0, NULL, FILE_END);
#else // INVALID_SET_FILE_POINTER doesn't seem to have a definition
    if (SetFilePointer(h, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
	goto err;
#endif

    if (WriteFile(h,buffer,len,&numwritten,NULL) != TRUE)
	goto err2;

    if (len != numwritten)
	goto err2;
    
    if (touchtime) {
        SetFileTime(h, NULL, NULL, &((WIN32_FIND_DATAA *)touchtime)->ftLastWriteTime);
    }
    if (!CloseHandle(h))
	goto err;
    return 0;

err2:
    CloseHandle(h);
err:
    return 1;
#else
    return 1;
#endif
}

/**************************************
 */

void File::readv()
{
    if (read())
	error("Error reading file '%s'\n",name->toChars());
}

/**************************************
 */

void File::mmreadv()
{
    if (mmread())
	readv();
}

void File::writev()
{
    if (write())
	error("Error writing file '%s'\n",name->toChars());
}

void File::appendv()
{
    if (write())
	error("Error appending to file '%s'\n",name->toChars());
}

/*******************************************
 * Return !=0 if file exists.
 *	0:	file doesn't exist
 *	1:	normal file
 *	2:	directory
 */

int File::exists()
{
#if _WIN32
    DWORD dw;
    int result;
    char *name;

    name = this->name->toChars();
    if (touchtime)
	dw = ((WIN32_FIND_DATAA *)touchtime)->dwFileAttributes;
    else
	dw = GetFileAttributesA(name);
    if (dw == -1L)
	result = 0;
    else if (dw & FILE_ATTRIBUTE_DIRECTORY)
	result = 2;
    else
	result = 1;
    return result;
#else
    return 0;
#endif
}

void File::remove()
{
#if _WIN32
    DeleteFileA(this->name->toChars());
#else
    ::remove(this->name->toChars());
#endif
}

Array *File::match(char *n)
{
    return match(new FileName(n, 0));
}

Array *File::match(FileName *n)
{
#if _WIN32
    HANDLE h;
    WIN32_FIND_DATAA fileinfo;
    Array *a;
    char *c;
    char *name;

    a = new Array();
    c = n->toChars();
    name = n->name();
    h = FindFirstFileA(c,&fileinfo);
    if (h != INVALID_HANDLE_VALUE)
    {
	do
	{
	    // Glue path together with name
	    char *fn;
	    File *f;

	    fn = (char *)mem.malloc(name - c + strlen(fileinfo.cFileName) + 1);
	    memcpy(fn, c, name - c);
	    strcpy(fn + (name - c), fileinfo.cFileName);
	    f = new File(fn);
	    f->touchtime = mem.malloc(sizeof(WIN32_FIND_DATAA));
	    memcpy(f->touchtime, &fileinfo, sizeof(fileinfo));
	    a->push(f);
	} while (FindNextFileA(h,&fileinfo) != FALSE);
	FindClose(h);
    }
    return a;
#else
    return NULL;
#endif
}

int File::compareTime(File *f)
{
#if _WIN32
    if (!touchtime)
	stat();
    if (!f->touchtime)
	f->stat();
    return CompareFileTime(&((WIN32_FIND_DATAA *)touchtime)->ftLastWriteTime, &((WIN32_FIND_DATAA *)f->touchtime)->ftLastWriteTime);
#else
    return 0;
#endif
}

void File::stat()
{
#if _WIN32
    HANDLE h;

    if (!touchtime)
    {
	touchtime = mem.calloc(1, sizeof(WIN32_FIND_DATAA));
    }
    h = FindFirstFileA(name->toChars(),(WIN32_FIND_DATAA *)touchtime);
    if (h != INVALID_HANDLE_VALUE)
    {
	FindClose(h);
    }
#else
    if (!touchtime)
    {
	touchtime = mem.calloc(1, sizeof(struct stat));
    }
#endif
}

void File::checkoffset(size_t offset, size_t nbytes)
{
    if (offset > len || offset + nbytes > len)
	error("Corrupt file '%s': offset x%zx off end of file",toChars(),offset);
}

char *File::toChars()
{
    return name->toChars();
}


/************************* OutBuffer *************************/

OutBuffer::OutBuffer()
{
    data = NULL;
    offset = 0;
    size = 0;
}

OutBuffer::~OutBuffer()
{
    mem.free(data);
}

void *OutBuffer::extractData()
{
    void *p;

    p = (void *)data;
    data = NULL;
    offset = 0;
    size = 0;
    return p;
}

void OutBuffer::mark()
{
    mem.mark(data);
}

void OutBuffer::reserve(unsigned nbytes)
{
    //printf("OutBuffer::reserve: size = %d, offset = %d, nbytes = %d\n", size, offset, nbytes);
    if (size - offset < nbytes)
    {
	size = (offset + nbytes) * 2;
	data = (unsigned char *)mem.realloc(data, size);
    }
}

void OutBuffer::reset()
{
    offset = 0;
}

void OutBuffer::setsize(unsigned size)
{
    offset = size;
}

void OutBuffer::write(const void *data, unsigned nbytes)
{
    reserve(nbytes);
    memcpy(this->data + offset, data, nbytes);
    offset += nbytes;
}

void OutBuffer::writebstring(unsigned char *string)
{
    write(string,*string + 1);
}

void OutBuffer::writestring(char *string)
{
    write(string,strlen(string));
}

void OutBuffer::writedstring(const char *string)
{
#if M_UNICODE
    for (; *string; string++)
    {
	writedchar(*string);
    }
#else
    write(string,strlen(string));
#endif
}

void OutBuffer::writedstring(const wchar_t *string)
{
#if M_UNICODE
    write(string,wcslen(string) * sizeof(wchar_t));
#else
    for (; *string; string++)
    {
	writedchar(*string);
    }
#endif
}

void OutBuffer::prependstring(char *string)
{   unsigned len;

    len = strlen(string);
    reserve(len);
    memmove(data + len, data, offset);
    memcpy(data, string, len);
    offset += len;
}

void OutBuffer::writenl()
{
#if _WIN32
#if M_UNICODE
    write4(0x000A000D);		// newline is CR,LF on Microsoft OS's
#else
    writeword(0x0A0D);		// newline is CR,LF on Microsoft OS's
#endif
#else
#if M_UNICODE
    writeword('\n');
#else
    writeByte('\n');
#endif
#endif
}

void OutBuffer::writeByte(unsigned b)
{
    reserve(1);
    this->data[offset] = (unsigned char)b;
    offset++;
}

void OutBuffer::writeUTF8(unsigned b)
{
    reserve(6);
    if (b <= 0x7F)
    {
	this->data[offset] = (unsigned char)b;
	offset++;
    }
    else if (b <= 0x7FF)
    {
	this->data[offset + 0] = (unsigned char)((b >> 6) | 0xC0);
	this->data[offset + 1] = (unsigned char)((b & 0x3F) | 0x80);
	offset += 2;
    }
    else if (b <= 0xFFFF)
    {
	this->data[offset + 0] = (unsigned char)((b >> 12) | 0xE0);
	this->data[offset + 1] = (unsigned char)(((b >> 6) & 0x3F) | 0x80);
	this->data[offset + 2] = (unsigned char)((b & 0x3F) | 0x80);
	offset += 3;
    }
    else if (b <= 0x1FFFFF)
    {
	this->data[offset + 0] = (unsigned char)((b >> 18) | 0xF0);
	this->data[offset + 1] = (unsigned char)(((b >> 12) & 0x3F) | 0x80);
	this->data[offset + 2] = (unsigned char)(((b >> 6) & 0x3F) | 0x80);
	this->data[offset + 3] = (unsigned char)((b & 0x3F) | 0x80);
	offset += 4;
    }
    else if (b <= 0x3FFFFFF)
    {
	this->data[offset + 0] = (unsigned char)((b >> 24) | 0xF8);
	this->data[offset + 1] = (unsigned char)(((b >> 18) & 0x3F) | 0x80);
	this->data[offset + 2] = (unsigned char)(((b >> 12) & 0x3F) | 0x80);
	this->data[offset + 3] = (unsigned char)(((b >> 6) & 0x3F) | 0x80);
	this->data[offset + 4] = (unsigned char)((b & 0x3F) | 0x80);
	offset += 5;
    }
    else if (b <= 0x7FFFFFFF)
    {
	this->data[offset + 0] = (unsigned char)((b >> 30) | 0xFC);
	this->data[offset + 1] = (unsigned char)(((b >> 24) & 0x3F) | 0x80);
	this->data[offset + 2] = (unsigned char)(((b >> 18) & 0x3F) | 0x80);
	this->data[offset + 3] = (unsigned char)(((b >> 12) & 0x3F) | 0x80);
	this->data[offset + 4] = (unsigned char)(((b >> 6) & 0x3F) | 0x80);
	this->data[offset + 5] = (unsigned char)((b & 0x3F) | 0x80);
	offset += 6;
    }
    else
	assert(0);
}

void OutBuffer::writedchar(unsigned b)
{
    reserve(Dchar_mbmax * sizeof(dchar));
    offset = (unsigned char *)Dchar::put((dchar *)(this->data + offset), (dchar)b) -
		this->data;
}

void OutBuffer::prependbyte(unsigned b)
{
    reserve(1);
    memmove(data + 1, data, offset);
    data[0] = (unsigned char)b;
    offset++;
}

void OutBuffer::writeword(unsigned w)
{
    reserve(2);
    *(unsigned short *)(this->data + offset) = (unsigned short)w;
    offset += 2;
}

void OutBuffer::writeUTF16(unsigned w)
{
    reserve(4);
    if (w <= 0xFFFF)
    {
	*(unsigned short *)(this->data + offset) = (unsigned short)w;
	offset += 2;
    }
    else if (w <= 0x10FFFF)
    {
	*(unsigned short *)(this->data + offset) = (unsigned short)((w >> 10) + 0xD7C0);
	*(unsigned short *)(this->data + offset + 2) = (unsigned short)((w & 0x3FF) | 0xDC00);
	offset += 4;
    }
    else
	assert(0);
}

void OutBuffer::write4(unsigned w)
{
    reserve(4);
    *(unsigned long *)(this->data + offset) = w;
    offset += 4;
}

void OutBuffer::write(OutBuffer *buf)
{
    if (buf)
    {	reserve(buf->offset);
	memcpy(data + offset, buf->data, buf->offset);
	offset += buf->offset;
    }
}

void OutBuffer::write(Object *obj)
{
    if (obj)
    {
	writestring(obj->toChars());
    }
}

void OutBuffer::fill0(unsigned nbytes)
{
    reserve(nbytes);
    memset(data + offset,0,nbytes);
    offset += nbytes;
}

void OutBuffer::align(unsigned size)
{   unsigned nbytes;

    nbytes = ((offset + size - 1) & ~(size - 1)) - offset;
    fill0(nbytes);
}

void OutBuffer::vprintf(const char *format, va_list args)
{
    char buffer[128];
    char *p;
    unsigned psize;
    int count;

    p = buffer;
    psize = sizeof(buffer);
    for (;;)
    {
#if _WIN32
	count = _vsnprintf(p,psize,format,args);
	if (count != -1)
	    break;
	psize *= 2;
#else
	count = vsnprintf(p,psize,format,args);
	if (count == -1)
	    psize *= 2;
	else if (count >= psize)
	    psize = count + 1;
	else
	    break;
#endif
	p = (char *) alloca(psize);	// buffer too small, try again with larger size
    }
    write(p,count);
}

#if M_UNICODE
void OutBuffer::vprintf(const wchar_t *format, va_list args)
{
    dchar buffer[128];
    dchar *p;
    unsigned psize;
    int count;

    p = buffer;
    psize = sizeof(buffer) / sizeof(buffer[0]);
    for (;;)
    {
#if _WIN32
	count = _vsnwprintf(p,psize,format,args);
	if (count != -1)
	    break;
	psize *= 2;
#else
	count = vsnwprintf(p,psize,format,args);
	if (count == -1)
	    psize *= 2;
	else if (count >= psize)
	    psize = count + 1;
	else
	    break;
#endif
	p = (dchar *) alloca(psize * 2);	// buffer too small, try again with larger size
    }
    write(p,count * 2);
}
#endif

void OutBuffer::printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format,ap);
    va_end(ap);
}

#if M_UNICODE
void OutBuffer::printf(const wchar_t *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format,ap);
    va_end(ap);
}
#endif

void OutBuffer::bracket(char left, char right)
{
    reserve(2);
    memmove(data + 1, data, offset);
    data[0] = left;
    data[offset + 1] = right;
    offset += 2;
}

/******************
 * Insert left at i, and right at j.
 * Return index just past right.
 */

unsigned OutBuffer::bracket(unsigned i, char *left, unsigned j, char *right)
{
    size_t leftlen = strlen(left);
    size_t rightlen = strlen(right);
    reserve(leftlen + rightlen);
    insert(i, left, leftlen);
    insert(j + leftlen, right, rightlen);
    return j + leftlen + rightlen;
}

void OutBuffer::spread(unsigned offset, unsigned nbytes)
{
    reserve(nbytes);
    memmove(data + offset + nbytes, data + offset,
	this->offset - offset);
    this->offset += nbytes;
}

/****************************************
 * Returns: offset + nbytes
 */

unsigned OutBuffer::insert(unsigned offset, const void *p, unsigned nbytes)
{
    spread(offset, nbytes);
    memmove(data + offset, p, nbytes);
    return offset + nbytes;
}

void OutBuffer::remove(unsigned offset, unsigned nbytes)
{
    memmove(data + offset, data + offset + nbytes, this->offset - (offset + nbytes));
    this->offset -= nbytes;
}

char *OutBuffer::toChars()
{
    writeByte(0);
    return (char *)data;
}

/********************************* Bits ****************************/

Bits::Bits()
{
    data = NULL;
    bitdim = 0;
    allocdim = 0;
}

Bits::~Bits()
{
    mem.free(data);
}

void Bits::mark()
{
    mem.mark(data);
}

void Bits::resize(unsigned bitdim)
{
    unsigned allocdim;
    unsigned mask;

    allocdim = (bitdim + 31) / 32;
    data = (unsigned *)mem.realloc(data, allocdim * sizeof(data[0]));
    if (this->allocdim < allocdim)
	memset(data + this->allocdim, 0, (allocdim - this->allocdim) * sizeof(data[0]));

    // Clear other bits in last word
    mask = (1 << (bitdim & 31)) - 1;
    if (mask)
	data[allocdim - 1] &= ~mask;

    this->bitdim = bitdim;
    this->allocdim = allocdim;
}

void Bits::set(unsigned bitnum)
{
    data[bitnum / 32] |= 1 << (bitnum & 31);
}

void Bits::clear(unsigned bitnum)
{
    data[bitnum / 32] &= ~(1 << (bitnum & 31));
}

int Bits::test(unsigned bitnum)
{
    return data[bitnum / 32] & (1 << (bitnum & 31));
}

void Bits::set()
{   unsigned mask;

    memset(data, ~0, allocdim * sizeof(data[0]));

    // Clear other bits in last word
    mask = (1 << (bitdim & 31)) - 1;
    if (mask)
	data[allocdim - 1] &= mask;
}

void Bits::clear()
{
    memset(data, 0, allocdim * sizeof(data[0]));
}

void Bits::copy(Bits *from)
{
    assert(bitdim == from->bitdim);
    memcpy(data, from->data, allocdim * sizeof(data[0]));
}

Bits *Bits::clone()
{
    Bits *b;

    b = new Bits();
    b->resize(bitdim);
    b->copy(this);
    return b;
}

void Bits::sub(Bits *b)
{
    unsigned u;

    for (u = 0; u < allocdim; u++)
	data[u] &= ~b->data[u];
}















