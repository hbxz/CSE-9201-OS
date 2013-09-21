#ifndef __FILE_H
#define __FILE_H

#include <types.h>
#include <synch.h>
#include <vnode.h>

// -----------------------------------------------
// file:	file.h
// author:	Frode Klevstul (frode@klevstul.com)
// date:	11.05.2003
// -----------------------------------------------




// -----
// setting up the filetable
// --
// filetable holding 3 files for stdin/out/error (0,1,2)
// and 16 other files => 18 (count with 0)
// defining an array to hold the actual vnodes
// and one array to hold the offset for each entry in the filetable
// There is a "filetable_dup" array, for storing infomation about
// duplicated fd's (when using dup2)
// We have an "filetable_isdup" array, which tells us if a
// fd is a duplicate. This is used for preventing closing dup's (only
// org. files can be closed)
// "filetable_parent" keeps track of which fd is the original for each dup
// --
#define FILETABLESIZE 18
struct vnode *filetable[FILETABLESIZE];
int filetable_offset[FILETABLESIZE];
int filetable_dup[FILETABLESIZE];
int filetable_isdup[FILETABLESIZE];
int filetable_parent[FILETABLESIZE];


// ---
// errorhandling
// ---
// errors found in file.c will be stored here, and can be read
// by syscall.c
// --
int file_errno;




// ---
// systemcalls
// ---
int write(int fd, const void *buf, size_t nbytes);

int open(const char *filename, int flags);

int close(int fd);

int read(int fd, void *buf, size_t buflen);

int lseek(int fd, off_t pos, int whence);

int __getcwd(char *buf, size_t buflen);

int chdir(const char *pathname);

int dup2(int oldfd, int newfd);




#endif

