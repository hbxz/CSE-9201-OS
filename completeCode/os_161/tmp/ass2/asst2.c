
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

/*
 * Modified by Frode Klevstul, for 
 * testing different cases not included
 * in the original asst2.c
 *
 */

#define MAX_BUF 500
char teststr[] = "The quick brown fox jumped over the lazy dog.";
char buf[MAX_BUF];
int
main(int argc, char * argv[])
{
  int fd, r, i, j , k;
  char * p;
  (void) argc;
  (void) argv;
  int read_buf;	// -- fk --
  int ftsize = 18; // -- fk --

  printf("\n**********\n* File Tester - Extended version\n");
  
  snprintf(buf, MAX_BUF, "**********\n* write() works for stdout\n");
  write(1, buf, strlen(buf));
  snprintf(buf, MAX_BUF, "**********\n* write() works for stderr\n");
  write(2, buf, strlen(buf));

  printf("**********\n* opening new file \"test.file\"\n");
  fd = open("test.file", O_RDWR | O_CREAT );
  printf("* open() got fd %d\n", fd);
  if (fd < 0) {
    printf("ERROR opening file: %s\n", strerror(errno));
    exit(1);
  }

  printf("* writing test string\n");  
  r = write(fd, teststr, strlen(teststr));
  printf("* wrote %d bytes\n", r);
  if (r < 0) {
    printf("ERROR writing file: %s\n", strerror(errno));
    exit(1);
  }

  printf("* writing test string again\n");  
  r = write(fd, teststr, strlen(teststr));
  printf("* wrote %d bytes\n", r);
  if (r < 0) {
    printf("ERROR writing file: %s\n", strerror(errno));
    exit(1);
  }
  printf("* closing file\n");  
  close(fd);
  
  printf("**********\n* opening old file \"test.file\"\n");
  fd = open("test.file", O_RDONLY);
  printf("* open() got fd %d\n", fd);
  if (fd < 0) {
    printf("ERROR opening file: %s\n", strerror(errno));
    exit(1);
  }

  printf("* reading entire file into buffer \n");
  i = 0;
  do  {  
  	printf("* attemping read of %d bytes\n", MAX_BUF -i);
  	r = read(fd, &buf[i], MAX_BUF - i);
	printf("* read %d bytes\n", r);
	i += r;
  } while (i < MAX_BUF && r > 0);
  
  printf("* reading complete\n");
  if (r < 0) {
    printf("ERROR reading file: %s\n", strerror(errno));
    exit(1);
  }
  k = j = 0;
  r = strlen(teststr);
  do {
  	if (buf[k] != teststr[j]) {
		printf("ERROR  file contents mismatch\n");
    		exit(1);
	}
	k++;
	j = k % r;
  } while (k < i);
	printf("* file content okay\n");

	printf("**********\n* testing lseek\n");
	r = lseek(fd, 5, SEEK_SET);
	if (r < 0) {
    		printf("ERROR lseek: %s\n", strerror(errno));
    	exit(1);
  	}
	
  printf("* reading 10 bytes of file into buffer \n");
  i = 0;
  do  {  
  	printf("* attemping read of %d bytes\n", 10 - i );
  	r = read(fd, &buf[i], 10 - i);
	printf("* read %d bytes\n", r);
	i += r;
  } while (i < 10 && r > 0);
  printf("* reading complete\n");
  if (r < 0) {
    printf("ERROR reading file: %s\n", strerror(errno));
    exit(1);
  }
 
  k = 0;
  j = 5;
  r = strlen(teststr);
  do {
  	if (buf[k] != teststr[j]) {
		printf("ERROR  file contents mismatch\n");
    		exit(1);
	}
	k++;
	j = (k + 5)% r;
  } while (k < 5);
	printf("* file lseek  okay\n");
   	printf("* closing file\n");  
  	close(fd);

	printf("**********\n* testing getcwd\n");

	p = getcwd(buf, MAX_BUF);
	
	if (p == NULL) {
		printf("ERROR  getcwd return NULL: %s\n", strerror(errno));
  		exit(1);
	}
	printf("* cwd = %s\n", buf);
	
	printf("**********\n* testing chdir \"emu0:include\"\n");
	
	r = chdir("emu0:include");
	if (r < 0) {
		printf("ERROR  chdir failed: %s\n", strerror(errno));
  		exit(1);
	}
	printf("* chdir succeeded\n");

	

	printf("**********\n* testing getcwd again\n");

	p = getcwd(buf, MAX_BUF);
	
	if (p == NULL) {
		printf("EXPECTED ERROR getcwd returned NULL: %s\n", strerror(errno));
	} 
	else {
		printf("* cwd = %s\n", buf);
	}
	
	printf("**********\n* Expecting to open \"assert.h\"\n");
	fd = open("assert.h", O_RDONLY);
  	printf("* open() got fd %d\n", fd);
  	if (fd < 0) {
    		printf("ERROR opening file: %s\n", strerror(errno));
    		exit(1);
  	}
	printf("* closing file\n");  
  	close(fd);


	// --- fk ---
	printf("*\n**********\n* ::: Extended testcases ::: \n*\n");

	fd = 8;
	printf("**********\n* Trying to write using a fd (%d) not opened\n",fd);
	printf("* writing test string\n");  
	r = write(fd, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("EXPECTED ERROR: %s\n", strerror(errno));
	}

	printf("**********\n* Trying to read using a fd (%d) not opened\n",fd);
  	printf("* attemping read of %d bytes\n", MAX_BUF);
  	r = read(fd, &buf[i], MAX_BUF);
	printf("* read %d bytes\n", r);
	if (r < 0) {
		printf("EXPECTED ERROR: %s\n", strerror(errno));
	}

	printf("**********\n* Trying to lseek using a fd (%d) not opened\n",fd);
	printf("* testing lseek\n");
	r = lseek(fd, 5, SEEK_SET);
	if (r < 0) {
    		printf("EXPECTED ERROR: %s\n", strerror(errno));
  	}

	printf("**********\n* Trying to lseek using an invalid 'whence'\n",fd);
	fd = open("test.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	printf("* testing lseek\n");
	r = lseek(fd, 5, 180876);
	if (r < 0) {
    		printf("EXPECTED ERROR: %s\n", strerror(errno));
  	}

	printf("**********\n* Trying to lseek resulting with a negative offset\n",fd);
	printf("* testing lseek\n");
	r = lseek(fd, -1, SEEK_SET);
	if (r < 0) {
    		printf("EXPECTED ERROR: %s\n", strerror(errno));
  	}

	printf("* closing file\n");  
	close(fd);

	fd = 2;
	printf("**********\n* Trying to lseek on a object not supporting seeking (stderr)\n",fd);
	printf("* testing lseek\n");
	r = lseek(fd, 5, SEEK_SET);
	if (r < 0) {
    		printf("EXPECTED ERROR: %s\n", strerror(errno));
  	}
	
	printf("**********\n* Testing wrong chdir \"fk:include\"\n");	
	r = chdir("fk:include");
	if (r < 0) {
		printf("EXPECTED ERROR: %s\n", strerror(errno));
	}

	printf("**********\n* Read 89 and then 90 bytes on a file with size 90...\n",fd);
	r = chdir("emu0:");
	if (r < 0) {
		printf("ERROR chdir failed: %s\n", strerror(errno));
  		exit(1);
	}
	fd = open("test.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	read_buf = 89;
  	printf("* attemping read of %d bytes\n", read_buf);
  	r = read(fd, &buf[i], read_buf);
  	if (r != read_buf) {
  		printf("ERROR read: expected to read %d bytes, not %d\n",read_buf,r);
  		exit(1);
  	}
	printf("* read %d bytes\n", r);

	read_buf = 90;
  	printf("* attemping read of %d bytes\n", read_buf);
  	r = read(fd, &buf[i], read_buf);
  	if (r != 1) {
  		printf("ERROR read: expected to read %d byte, not %d\n",1,r);
  		exit(1);
  	}
	printf("* read %d bytes\n", r);


	printf("**********\n* Find EOF (lseek) and read 90 bytes\n",fd);
	printf("* testing lseek\n");
	r = lseek(fd, 0, SEEK_END);
	if (r < 0) {
    	printf("ERROR lseek: %s\n", strerror(errno));
	  	exit(1);
  	}
	read_buf = 90;
  	printf("* attemping read of %d bytes\n", read_buf);
  	r = read(fd, &buf[i], read_buf);
  	if (r != 1) {
  		printf("ERROR read: expected to read %d byte, not %d\n",1,r);
  		exit(1);
  	}
	printf("* read %d bytes\n", r);

	printf("* closing file\n");  
  	close(fd);

	fd = 8;
	printf("**********\n* Trying to close an unopened fd (%d)\n",fd);
	printf("* closing file\n");  
  	r = close(fd);
  	if (r) {
    	printf("EXPECTED ERROR: %s\n", strerror(errno));
  	} else {
    	printf("ERROR: it should not be possible to close this file\n");
    	exit(1);
  	}

	fd = 2;
	printf("**********\n* Trying to close stderr (%d)\n",fd);
	printf("* closing file\n");  
  	r = close(fd);
  	if (r) {
    	printf("EXPECTED ERROR: %s\n", strerror(errno));
  	} else {
    	printf("ERROR: it should not be possible to close this file\n");
    	exit(1);
  	}

	fd = 1;
	printf("**********\n* Trying to read from stdout (%d)\n",fd);
  	printf("* attemping read of %d bytes\n", MAX_BUF);
  	r = read(fd, &buf[i], MAX_BUF);
	printf("* read %d bytes\n", r);
	if (r < 0) {
		printf("EXPECTED ERROR: %s\n", strerror(errno));
  	} else {
    	printf("ERROR: it should not be possible to read from stdout\n");
    	exit(1);
  	}

	fd = 0;
	printf("**********\n* Trying to write to stdin (%d)\n",fd);
	printf("* writing test string\n");  
	r = write(fd, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("EXPECTED ERROR: %s\n", strerror(errno));
  	} else {
    	printf("ERROR: it should not be possible to write to stdin\n");
    	exit(1);
  	}


	printf("**********\n* Trying to open too many files, wihtout closing them...\n",fd);
	fd = open("test1.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test2.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test3.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test4.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test5.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test6.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test7.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test8.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test9.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test10.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test11.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test12.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test13.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test14.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test15.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test16.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test17.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("EXCPECTED ERROR: %s\n", strerror(errno));
	}
	fd = open("test18.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("EXCPECTED ERROR: %s\n", strerror(errno));
	}

	for (fd=3; fd<(ftsize+1); fd++){
		printf("Closing file %d\n",fd);
		close(fd);
	}


	printf("**********\n* Trying to open 'test1.file' twice, wihtout closing it in between...\n",fd);
	fd = open("test1.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	fd = open("test1.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}

	printf("* Closing files...\n",fd);
	for (fd=3; fd<(ftsize+1); fd++){
		close(fd);
	}



	printf("**********\n* Testing dup2()\n",fd);
	fd = open("test1.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}

	k = 8;
	printf("* duplicates old fd %d with new fd %d-> dup2(%d,%d)\n", fd, k, fd, k);
	r = dup2(fd,k);
	if (r < 0) {
		printf("ERROR duplicating file: %s\n", strerror(errno));
		exit(1);
	}

	printf("* write test string into %d\n", k);
	r = write(k, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("ERROR writing file: %s\n", strerror(errno));
		exit(1);
	}

	printf("* write test string into %d\n", fd);
	r = write(fd, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("ERROR writing file: %s\n", strerror(errno));
		exit(1);
	}
	
	printf("* Closing files %d and %d...\n",k,fd);
	r = close(k);
	if (r < 0) {
		printf("EXPECTED ERROR (can't close duplicate fd=%d): %s\n", k, strerror(errno));
	}
	r = close(fd);
	if (r < 0) {
		printf("ERROR closing file: %s\n", strerror(errno));
		exit(1);
	}

	fd = open("test.file", O_RDONLY);
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	
	printf("* reading entire file into buffer \n");
	i = 0;
	do  {  
		printf("* attemping read of %d bytes\n", MAX_BUF -i);
		r = read(fd, &buf[i], MAX_BUF - i);
		printf("* read %d bytes\n", r);
		i += r;
	} while (i < MAX_BUF && r > 0);
	
	printf("* reading complete\n");
	if (r < 0) {
		printf("ERROR reading file: %s\n", strerror(errno));
		exit(1);
	}
	k = j = 0;
	r = strlen(teststr);
	do {
	if (buf[k] != teststr[j]) {
		printf("ERROR  file contents mismatch\n");
			exit(1);
	}
	k++;
	j = k % r;
	} while (k < i);
		printf("* file content okay\n");

	printf("* closing file\n");  
	close(fd);


	printf("**********\n* Advanced testing of dup2()\n",fd);
	fd = open("test1.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}

	k = 4;
	printf("* duplicates old fd %d with new fd %d-> dup2(%d,%d)\n", fd, k, fd, k);
	r = dup2(fd,k);
	if (r < 0) {
		printf("ERROR duplicating file: %s\n", strerror(errno));
		exit(1);
	}

	i = open("test1.file", O_RDWR | O_CREAT );
	printf("* open() got fd %d\n", i);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	} else if (fd==i) {
		printf("ERROR opening file: fd=%d is a duplicate %s\n", i, strerror(errno));
		exit(1);
	}

	printf("* close fd=%d\n",i);
	close(i);


	k = 8;
	printf("* duplicates old fd %d with new fd %d-> dup2(%d,%d)\n", fd, k, fd, k);
	r = dup2(fd,k);
	if (r < 0) {
		printf("ERROR duplicating file: %s\n", strerror(errno));
		exit(1);
	}

	j = 18;
	printf("* duplicates old fd %d with new fd %d-> dup2(%d,%d)\n", fd, j, fd, j);
	r = dup2(fd,j);
	if (r < 0) {
		printf("ERROR duplicating file: %s\n", strerror(errno));
		exit(1);
	}

	i = 16;
	printf("* duplicates old fd %d with new fd %d-> dup2(%d,%d)\n", fd, i, fd, i);
	r = dup2(fd,i);
	if (r < 0) {
		printf("ERROR duplicating file: %s\n", strerror(errno));
		exit(1);
	}


	printf("* write test string into %d\n", fd);
	r = write(fd, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("ERROR writing file: %s\n", strerror(errno));
		exit(1);
	}

	printf("* write test string into %d\n", i);
	r = write(i, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("ERROR writing file: %s\n", strerror(errno));
		exit(1);
	}
	
	printf("* write test string into %d\n", j);
	r = write(j, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("ERROR writing file: %s\n", strerror(errno));
		exit(1);
	}

	printf("* write test string into %d\n", k);
	r = write(k, teststr, strlen(teststr));
	printf("* wrote %d bytes\n", r);
	if (r < 0) {
		printf("ERROR writing file: %s\n", strerror(errno));
		exit(1);
	}


	printf("* Closing files, dup %d and original file %d...\n",j,fd);
	r = close(j);
	if (r < 0) {
		printf("EXPECTED ERROR (can't close duplicate fd=%d): %s\n", j, strerror(errno));
	}
	r = close(fd);
	if (r < 0) {
		printf("ERROR closing file: %s\n", strerror(errno));
		exit(1);
	}

	fd = open("test.file", O_RDONLY);
	printf("* open() got fd %d\n", fd);
	if (fd < 0) {
		printf("ERROR opening file: %s\n", strerror(errno));
		exit(1);
	}
	
	printf("* reading entire file into buffer \n");
	i = 0;
	do  {  
		printf("* attemping read of %d bytes\n", MAX_BUF -i);
		r = read(fd, &buf[i], MAX_BUF - i);
		printf("* read %d bytes\n", r);
		i += r;
	} while (i < MAX_BUF && r > 0);
	
	printf("* reading complete\n");
	if (r < 0) {
		printf("ERROR reading file: %s\n", strerror(errno));
		exit(1);
	}
	k = j = 0;
	r = strlen(teststr);
	do {
	if (buf[k] != teststr[j]) {
		printf("ERROR  file contents mismatch\n");
			exit(1);
	}
	k++;
	j = k % r;
	} while (k < i);
		printf("* file content okay\n");

	printf("* closing file\n");  
	close(fd);



	// --- /fk ---

  	return 0;
}


