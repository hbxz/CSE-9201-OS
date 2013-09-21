/*
 * Program to test argument passing: it displays the argc and all
 * of the argv, and then exit.
 *
 * Intended for assignment 2. This may help debugging the argument
 * handling of execv().
 */

#include <stdio.h>

int
main(int argc, char *argv[])
{
	const char *tmp;
	int i;

	printf("argc: %d\n", argc);

	for (i=0; i<=argc; i++) {
		tmp = argv[i];
		if (tmp==NULL) {
			tmp = "[NULL]";
		}
		printf("argv[%d]: %s\n", i, tmp);
	}

	return 0;
}
