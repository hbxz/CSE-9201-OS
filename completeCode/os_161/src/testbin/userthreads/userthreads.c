/*
 * test multiple user level threads inside a process the program forks
 * 3 threads off 2 to procedures which display a string every once in
 * a while
 *
 * This won't do much of anything unless you implement user-level
 * threads as part of asst5.
 */


#include <unistd.h>
#include <stdio.h>

#define NTHREADS  3
#define MAX       1<<25

/* counter for the loop in the threads : 
   This variable is shared and incremented by each 
   thread during his computation */
volatile int count = 0;

/* the 2 threads : */
void ThreadRunner(void);
void BladeRunner(void);

int
main(int argc, char *argv[])
{
    int i;

    (void)argc;
    (void)argv;

    for (i=0; i<NTHREADS; i++) {
	if (i)
	    threadfork(ThreadRunner);
        else
	    threadfork(BladeRunner);
    }

    printf("Parent has left.\n");
    return 0;
}

/* multiple threads will simply print out the global variable.
   Even though there is no synchronization, we should get some 
   random results.
*/

void
BladeRunner()
{
    while (count < MAX) {
	if (count % 500 == 0)
	    printf("Blade ");
	count++;
    }
}

void
ThreadRunner()
{
    while (count < MAX) {
	if (count % 513 == 0)
	    printf(" Runner\n");
	count++;
    }
}
    
