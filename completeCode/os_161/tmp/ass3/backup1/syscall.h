#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);


int sys_open(userptr_t filename, int flags, int mode, int *retval);
int sys_kopen(const char * filename, int flags, int fd, int *retval);

int sys_read(int filehandle, userptr_t buf, size_t size, int *retval);
int sys_write(int filehandle, userptr_t buf, size_t size, int *retval);
int sys_close(int filehandle);
int sys_lseek(int filehandle, off_t pos, int code, int * retval);

// -- fk --
int sys_fstat(int filehandle, userptr_t buf);
// -- / --

#endif /* _SYSCALL_H_ */
