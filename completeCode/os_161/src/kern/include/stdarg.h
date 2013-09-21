#ifndef _STDARG_H_
#define _STDARG_H_

/* Get the real stuff, which is machine-dependent */
#include <machine/stdarg.h>

/* Make sure we have __PF() */
#include <lib.h>

/*
 * The v... versions of printf functions in <lib.h>. This is not really
 * the best place for them...
 */
void vkprintf(const char *fmt, va_list ap) __PF(1,0);
int vsnprintf(char *buf, size_t maxlen, const char *f, va_list ap) __PF(3,0);

/*
 * The printf driver function (shared with libc).
 * Does v...printf, passing the output data piecemeal to the function
 * supplied. The "clientdata" argument is passed through to the function.
 * The strings passed to the function may not be null-terminated.
 */
int __vprintf(void (*func)(void *clientdata, const char *str, size_t len), 
	      void *clientdata, const char *format, va_list ap) __PF(3,0);

#endif /* _STDARG_H_ */
