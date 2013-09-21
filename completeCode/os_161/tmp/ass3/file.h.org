#ifndef __FILE_H
#define __FILE_H

#include <types.h>
#include <synch.h>
#include <vnode.h>

#define MAX_OPEN_FILES 64
#define MAX_FD 16


#define BUSY_OFTE ((void *)-1)

struct ofte {
  off_t fp;
  int flags;
  int count;
  struct vnode *vn;
};
	


struct fdt {
	struct ofte *fd[MAX_FD];
};

		
extern void oft_bootstrap(void);
extern struct ofte oft[];

#endif

