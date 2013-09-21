#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <limits.h>

/*
 * pwd - print working directory.
 * Usage: pwd
 *
 * Just uses the getcwd library call (which in turn uses the __getcwd
 * system call.)
 */

int
main()
{
	char buf[PATH_MAX+1], *p;
	int len;
	p = getcwd(buf, sizeof(buf)-1);
	if (p == NULL) {
		err(1, ".");
	}
	len = strlen(buf);
	buf[len] = 0;
	printf("%s\n", buf);
	return 0;
}
