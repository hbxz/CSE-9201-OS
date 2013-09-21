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

//#include <fs.h> //fk
//#include <kern/sfs.h> //fk

struct ofte oft[MAX_OPEN_FILES];

void mk_uio(struct uio *uio, userptr_t ubuf, size_t len, off_t pos, 
	    enum uio_rw rw)
{
  uio->uio_iovec.iov_ubase = ubuf;
  uio->uio_iovec.iov_len = len;
  uio->uio_offset = pos;
  uio->uio_resid = len;
  uio->uio_segflg = UIO_USERSPACE;
  uio->uio_rw = rw;
  uio->uio_space = curthread->t_vmspace;
}

enum fd_access {F_READ, F_WRITE, F_ANY};

int check_fd_access(int filehandle, struct fdt *fd_table, 
		    enum fd_access acc) 
{
  
  /* check file handle */
  if (filehandle < 0 || filehandle >= MAX_FD) {
    return EBADF;
  }
  if (fd_table->fd[filehandle] == NULL) {
    return EBADF;
  }

  switch (fd_table->fd[filehandle]->flags & O_ACCMODE) {
  case O_RDONLY:
    if (acc == F_WRITE) return EBADF;
    break;
  case O_WRONLY:
    if (acc == F_READ) return EBADF;
    break;
  case O_RDWR:
    break;
  default:
    panic("expected access mode in open file table entry\n");
  }
  return 0;
}

struct ofte *oft_alloc(void)
{
  int i;
  /* allocate a oft entry */
  for (i = 0; i < MAX_OPEN_FILES; i++) {
    if (oft[i].vn == NULL) {
      oft[i].vn = BUSY_OFTE;
      oft[i].count = 1;
      return &oft[i];
    }
  }
  return NULL;
}

int oft_free(struct ofte *ofptr) 
{
  int c;
  ofptr->count --;
  c =  ofptr->count;
  if (c == 0) {
    ofptr->vn = NULL;
  }
  return c;
}


void oft_bootstrap(void)
{
	int i;
	/* init the table */
	for (i = 0; i < MAX_OPEN_FILES; i++) {
		oft[i].vn = NULL;
	}
		
}


int sys_open(userptr_t filename, int flags, int mode, int *retval)
{
  static char pathname[PATH_MAX];
  int r,i, fd;
  size_t len;
  struct ofte *oftp;
  struct vnode *vnret;
  
  /* ignore mode */
  (void) mode;
  
  /* copy the pathname into a local buffer */
  r = copyinstr(filename, pathname, PATH_MAX, &len);
  if (r) {
    return r;
  }
  
  /* no synch protection needed as only current thread accesses */
  /* allocate a fd entry */
  
  for (i = 0; i < MAX_FD; i ++){
    if (curthread->t_fdt->fd[i] == NULL) {
      fd = i;
      break;
    }
  }
  if (i == MAX_FD) {
    return EMFILE;
  }
			
  oftp = oft_alloc();
  if (oftp == NULL) {
    curthread->t_fdt->fd[fd] = NULL;
    return ENFILE;
  }
		
  /* call  vfs open */
  r = vfs_open(pathname, flags, &vnret);
  if (r) {
    oft_free(oftp);
    curthread->t_fdt->fd[fd] = NULL;
    return r;
  }
  /* fill in the state */
  curthread->t_fdt->fd[fd] = oftp;
  oftp->vn = vnret;
  oftp->fp = 0;
  oftp->flags = flags;
  *retval = fd;
	
  /* return */
  return 0;  
}


int sys_kopen(const char *filename, int flags, int fd, int *retval)
{
  char *pathname;
  int r;
  struct ofte *oftp;
  struct vnode *vnret;
  
  
  /* no synch protection needed as only current thread accesses */
  /* allocate a fd entry */
  if (curthread->t_fdt->fd[fd] != NULL) {
    return EMFILE;
  }
  
  
  /* allocate a oft entry */
  oftp = oft_alloc();
  if (oftp == NULL) {
    curthread->t_fdt->fd[fd] = NULL;
    return ENFILE;
  }
  
  /* call  vfs open */
  pathname = kstrdup(filename);
  r = vfs_open(pathname, flags, &vnret);
  kfree(pathname);
  
  if (r) {
    oft_free(oftp);
    curthread->t_fdt->fd[fd] = NULL;
    return r;
  }
  /* fill in the state */
  curthread->t_fdt->fd[fd] = oftp;
  oftp->vn = vnret;
  oftp->fp = 0;
  oftp->flags = flags;
  *retval = fd;
  
  /* return */
  return 0;  
}

int sys_read(int filehandle, userptr_t buf, size_t size, int *retval)
{
	struct uio io;
	int r;
	
	/* check file handle */
	r = check_fd_access(filehandle, curthread->t_fdt, F_READ);
	if (r) return r;
	
	/* build uio */
	mk_uio(&io, buf, size, 
	       curthread->t_fdt->fd[filehandle]->fp,
	       UIO_READ);
	
	r = VOP_READ(curthread->t_fdt->fd[filehandle]->vn, &io);
	if (r) {
		return r;
	}
	*retval = io.uio_offset - curthread->t_fdt->fd[filehandle]->fp;
	curthread->t_fdt->fd[filehandle]->fp = io.uio_offset;

	return 0;	
}

int sys_write(int filehandle, userptr_t buf, size_t size, int *retval)
{
  	struct uio io;
	int r;
	
	r = check_fd_access(filehandle, curthread->t_fdt, F_WRITE);
	if (r) return r;
	
	mk_uio(&io, buf, size, 
	       curthread->t_fdt->fd[filehandle]->fp,
	       UIO_WRITE);
	
	r = VOP_WRITE(curthread->t_fdt->fd[filehandle]->vn, &io);
	if (r) {
	  return r;
	}
	*retval = io.uio_offset - curthread->t_fdt->fd[filehandle]->fp;
	curthread->t_fdt->fd[filehandle]->fp = io.uio_offset;
	
	return 0;	
}

int sys_close(int filehandle)
{
  struct vnode *vn;
  int r;
	
  r = check_fd_access(filehandle, curthread->t_fdt, F_ANY);
  if (r) return r;
  
  vn = curthread->t_fdt->fd[filehandle]->vn;
  r = oft_free(curthread->t_fdt->fd[filehandle]);
  if (r == 0)
    vfs_close(vn);
  
  /* free the oft entry and fd entry */
  curthread->t_fdt->fd[filehandle] = NULL;
  return 0;
}

int sys_lseek(int filehandle, off_t pos, int code, int *retval)
{
	off_t newfp;
	int r;
	struct stat statbuf;
  	/* check file handle */
	r = check_fd_access(filehandle, curthread->t_fdt, F_ANY);
	if (r) return r;
	
	switch (code) {
	case SEEK_SET:
		newfp = pos;
		break;
			
	case SEEK_CUR:
		newfp = curthread->t_fdt->fd[filehandle]->fp + pos; 
		break;
	case SEEK_END:
		r = VOP_STAT(curthread->t_fdt->fd[filehandle]->vn, &statbuf);
		if (r) return r;
		newfp = statbuf.st_size + pos; 
		break;
	default:
		return EINVAL;
	}
	
	if (newfp < 0) {
		return EINVAL;
	}
	
	r = VOP_TRYSEEK(curthread->t_fdt->fd[filehandle]->vn, newfp);
	if (r < 0) {
	  return r;
	}
	curthread->t_fdt->fd[filehandle]->fp = newfp;
	*retval = newfp;
	return 0;
}


// -- fk --
int sys_fstat(int filehandle, userptr_t buf)
{
	int r;
	struct uio io;
	struct stat statbuf;


	kprintf("filehandle: %d\n",filehandle);

	// 5606198 -rw-r-----    1 fbkl835  fbkl835   1049088 May 24 16:58 DISK1.img
	// <Q>: is the vnode here representing the entire mounted disk (DISK1.IMG)?
	// in kernel file stats: size: 73216

	// Or do we have to change the implementation of VOP_STAT to get this to work
	// properly?
	
	// <Q>: How to include functions in "sfs_vnode.c" (like "sfs_stat")? Shall this 
	//		be done in file.c? syscall.c? Which header file to include?

//vfs stat function 
//- bmap for block modify this
// count in allocation/deallocation	

	/* check file handle */
	r = check_fd_access(filehandle, curthread->t_fdt, F_READ);
	if (r) return r;
	
	/* build uio */
	mk_uio(&io, buf, sizeof(struct stat), curthread->t_fdt->fd[filehandle]->fp, UIO_READ);

//	r = VOP_STAT(curthread->t_fdt->fd[filehandle]->vn, &statbuf);
//	if (r) return r;

	r = fstat(curthread->t_fdt->fd[filehandle]->vn, &statbuf);
	if (r) return r;

// look for: sfs_stat in header files! Include that in this file?
// #include sfs_vnode.c

//	r = sfs_stat(filehandle, &statbuf);
//	if (r) return r;

	kprintf("in kernel file stats: size: %d blocks: %d stmode: %d \n", statbuf.st_size,statbuf.st_blocks, statbuf.st_mode);


	// <Q> How to transform a statbuf into a uio? Have to do this to be able to copy
	//     the data into userspace?
	
	
	return 0;

}
// -- / --



