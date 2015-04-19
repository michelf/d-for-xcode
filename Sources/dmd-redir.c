#include <unistd.h>

int main(unsigned int argc, char **argv) {
	argv[0] = "/usr/local/dmd/osx/bin/dmd";
	execv("/usr/local/dmd/osx/bin/dmd", argv);
}