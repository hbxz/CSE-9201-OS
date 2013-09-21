/*
 * forkbomb - apply malthus to an operating system ;-)
 *
 * DO NOT RUN THIS ON A REAL SYSTEM - IT WILL GRIND TO A HALT AND
 * PEOPLE WILL COME AFTER YOU WIELDING BASEBALL BATS OR THE AD
 * BOARD. WE WARNED YOU.
 *
 * (We don't expect your system to withstand this without grinding to
 * a halt, but once asst2 is complete it shouldn't crash. At least in
 * an ideal world. If it crashes your system in a way that's hard to
 * fix/avoid, talk to the course staff.)
 */

#include <unistd.h>
#include <err.h>

static volatile int pid;

int
main()
{
	int i;

	while (1) {
		fork();

		pid = getpid();

		/* Make sure each fork has its own address space. */
		for (i=0; i<300; i++) {
			volatile int seenpid;
			seenpid = pid;
			if (seenpid != getpid()) {
				errx(1, "pid mismatch (%d, should be %d) "
				     "- your vm is broken!", 
				     seenpid, getpid());
			}
		}
	}
}
