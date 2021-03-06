#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <file.h>
#include <vfs.h>
#include <vnode.h>
#include <lib.h>
#include <kern/limits.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/stat.h>
#include <uio.h>

// -----------------------------------------------
// file:	file.c
// author:	Frode Klevstul (frode@klevstul.com)
// date:	11.05.2003
// -----------------------------------------------




int write(int fd, const void *buf, size_t nbytes){
	int retval = 0;				// return value
	struct uio ku;				// kernel uio struct
	void *kbuffer;				// kernel buffer
	kbuffer = kmalloc(nbytes);	// allocate the buffer

	// stdin should be read-only
	if (fd==0){
		file_errno = EBADF;
		retval = -1;
	// we have to check that fd is valid (not NULL)
	} else if ( filetable[fd]!=NULL ){

		// copy the data from userspace to kernelspace
		file_errno = copyin( (const_userptr_t)buf, kbuffer, nbytes);
		if (file_errno)
			return -1;

		// set up kernel uio for kernel I/O
		mk_kuio(&ku, kbuffer, nbytes, filetable_offset[fd], UIO_WRITE);

		// write data (which now is in kernel) to the vnode in the filetable
		file_errno = VOP_WRITE(filetable[fd],&ku);
		if (file_errno)
			return -1;

		// update the filetable offset
		filetable_offset[fd] += nbytes;

		// update offset for all duplicates
		updateoffset(fd);

		// returning the number of bytes we have written
		retval = nbytes;		
	} else {
		// fd is not a valid file descriptor (out of range)
		file_errno = EBADF;
		retval = -1;
	}

	return retval;
}




int open(const char *filename, int flags){
	int retval = 0;	// return value
	int i;			// counter

	// go through filetable from bottom, look for an available slot
	for (i=3;i<=FILETABLESIZE;i++){
		if (filetable[i]==NULL && filetable_isdup[i]==NULL){
			// first time you open the file set offset to zero
			// and allocate some memory
			filetable_offset[i] = 0;
			filetable_dup[i] = -1;			// this fd don't have any duplicates
			filetable_isdup[i] = NULL;		// this is not a duplicate, it's a new file
			filetable_parent[i] = NULL;		// this file hasn't got any parent (it's not a dup)
			filetable[i] = kmalloc(sizeof(struct vnode));
			file_errno = vfs_open(filename, flags, &filetable[i]);
			if (file_errno)
				return -1;
			else
				retval = i;
				break;
		}
	}

	// The file-table is full (didn't find empty slot)
	if (i>FILETABLESIZE){
		file_errno = EMFILE;
		retval = -1;
	}

	return retval;
}




int close(int fd){
	int retval = 0;
	int i;

	// not possible to close stdin,stdout,stderr
	if (fd==0 || fd==1 || fd==2){
		retval = -1;
		file_errno = EBADF;
	} else if (filetable[fd]!=NULL && filetable_isdup[fd]==NULL) {
		// close all duplicates for this fd
		for (i=0; i<=FILETABLESIZE; i++){
			if (filetable_parent[i]==fd){
				filetable_isdup[i]=NULL;
				filetable_parent[i]=NULL;
				filetable[i]=NULL;
			}
		}

		// clean up: vfs_close() never fails. 
		// Free memory and set offset to null.
		vfs_close(filetable[fd]);
		filetable[fd] = NULL;
		kfree(filetable[fd]);
		filetable_offset[fd]=-1;
		filetable_dup[fd]=-1;

		retval = 0;
	} else {
		retval = -1;
		file_errno = EBADF;
	}

	return retval;
}




int read(int fd, void *buf, size_t buflen){
	int retval = 0;				// returnvalue
	struct uio ku;				// _K_ernelst_U_ff, a uio used for datareading
	struct stat filestat;		// statistic for our file
	void *kbuffer;				// kernelbuffer, we read data into this
	kbuffer = kmalloc(buflen);	// have to allocate some space for our new buffer
	int read_length;

	// check if fd is a valid file handler.
	if ( filetable[fd] == NULL){
		file_errno = EBADF;
		retval = -1;
	// stdout and stderr is write only
	} else if ( fd==1 || fd==2 ) {
		file_errno = EBADF;
		retval = -1;
	} else {

		// select out som statistics for this file
		VOP_STAT(filetable[fd], &filestat);

		// check that we have some bytes to read
		if ( filetable_offset[fd] < filestat.st_size ){

			// if we try to read more bytes than exisist in the file,
			// then set readlength to the maximum we can read
			if (buflen > filestat.st_size - filetable_offset[fd]){
				read_length = filestat.st_size - filetable_offset[fd];
			} else {
				read_length = buflen;
			}

			// set up uio for kernel I/O and read bytes into it
			mk_kuio(&ku, kbuffer, read_length, filetable_offset[fd], UIO_READ);
			file_errno = VOP_READ(filetable[fd], &ku);
			if (file_errno)
				return -1;
	
			// update offset
			filetable_offset[fd] += read_length;

			// update offset for all duplicates
			updateoffset(fd);

			// copyout to userspace
			copyout(kbuffer, (userptr_t)buf, buflen);
	
			// return number of bytes read
			retval = read_length;

		// don't have anything more to read, return 0 bytes...
		} else {
			retval = 0;
		}
	}

	return retval;
}




int lseek(int fd, off_t pos, int whence){
	int retval = 0;			// return value
	int old_offset;			// saves old offset in case of error
	struct stat filestat;	// get stat of file

	// stdin, stdout, stderr does not support seeking (I guess)
	if (fd==0 || fd==1 || fd==2){
		file_errno = ESPIPE;
		retval = -1;
	// Out of range? Enough to check if filetable is NULL,  because if
	// fd is out of range the filetable entry will always be NULL
	} else if (filetable[fd]==NULL){
		file_errno = EBADF;
		retval = -1;
	// everything looks great, so far...
	} else {
		// saves the old offset in case we get a negative offset
		old_offset = filetable_offset[fd];

		// the new position is pos
		if (whence ==  SEEK_SET){
			filetable_offset[fd]=pos;
		// the new position is the current position plus pos
		} else if (whence == SEEK_CUR){
			filetable_offset[fd]+=pos;
		// the new position is the position of end-of-file plus pos
		} else if (whence == SEEK_END) {
			VOP_STAT(filetable[fd], &filestat);
			filetable_offset[fd] = (filestat.st_size-1) + pos;
		} else {
			file_errno = EINVAL;
			retval = -1;
		}

		// check if this new position is legal, on a legal file
		file_errno = VOP_TRYSEEK(filetable[fd],filetable_offset[fd]);
		if (file_errno){
			filetable_offset[fd] = old_offset;
			retval = -1;
		}

	}


	// update return value if we don't have an error
	if (retval==0){
		// check result-seeking position is negative
		if ( filetable_offset[fd]<0){
			// resets offset and return error
			filetable_offset[fd] = old_offset;
			file_errno = EINVAL;
			retval = -1;
		} else {
			retval = filetable_offset[fd];
		}
	}

	return retval;
}




int __getcwd(char *buf, size_t buflen){
	int retval = 0;				// return value
	struct uio ku;				// kernel uio
	void *kbuffer;				// kernel buffer
	kbuffer = kmalloc(buflen);

	mk_kuio(&ku, kbuffer, buflen, 0, UIO_READ);
	file_errno = vfs_getcwd(&ku);
	if (file_errno)
		return -1;

	// returning the size of the wd-string
	retval = ku.uio_iovec.iov_len;

	// copy out to userspace
	copyout(kbuffer, (userptr_t)buf, buflen);

	return retval;
}




int chdir(const char *pathname){
	int retval = 0;

	file_errno = vfs_chdir(pathname);
	if (file_errno)
		return -1;

	return retval;
}




int dup2(int oldfd, int newfd){
	int retval = 0;
	int orgoldfd = oldfd;	// saves the "original" oldfd
	int go = 1;

	// fd out of range
	if ( (oldfd<0 || oldfd>FILETABLESIZE) || (newfd<0 || newfd>FILETABLESIZE) ){
		file_errno = EBADF;
		retval = -1;
	} else {

		if (filetable[newfd]!=NULL){
			file_errno = close(newfd);
			if (file_errno)
				return -1;
		}
		
		// there has to be an entry in filetable[oldfd]
		if ( filetable[oldfd] == NULL ){
			file_errno = EBADF;
			retval = -1;
		} else {

			// check if we already have one dup of this oldfd.
			// this new "copy" will be a duplicate of the duplicate
			// (of the duplicate etc..) this is is to provide 
			// "updateoffset()" will work properly. this is a linked list
			// where the first node (parent) is the original fd
			while (filetable_dup[oldfd]>0 && go){
				// have to prevent inifinte passage...
				if (orgoldfd == filetable_dup[oldfd]){
					go = NULL;
				} else { 
					oldfd = filetable_dup[oldfd];
				}
			}

			filetable[newfd] = filetable[oldfd];
			// update linked list
			filetable_dup[newfd] = orgoldfd;
			filetable_dup[oldfd] = newfd;
			// it's a duplicate
			filetable_isdup[newfd] = 1;
			// set's this dup's original fd
			filetable_parent[newfd] = orgoldfd;
			retval = 0;
		}
	}

	return retval;
}



void updateoffset(int fd){
	int i;
	int orgfd = fd;
	int go = 1;
	
	
	// go through our linked list of duplicates
	while (filetable_dup[fd]>0 && go){

		filetable_offset[filetable_dup[fd]]= filetable_offset[fd];

		if (orgfd == filetable_dup[fd]){
			go = NULL;
		} else { 
			fd = filetable_dup[fd];
		}
	}

}



