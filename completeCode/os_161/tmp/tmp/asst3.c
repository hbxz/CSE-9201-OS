
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

#define MAX_BUF 128

char teststr1[] = "The quick brown fox jumped over the lazy dog.";
char teststr2[] = "Hey, diddle diddle, the cat played the fiddle.";

char buf[MAX_BUF];


/*
 * A routine to:
 *      write to specified position in a file
 *      lseek back to that position
 *      read the contents back
 *      compare to see if contents are there
 */

int write_read_test(const char *filename, const char *str, off_t pos) 
{
  int fd, len, i, r;
  struct stat statbuf;
  
  printf("opening file \"%s\"\n", filename);
  fd = open(filename, O_RDWR | O_CREAT );
  printf("open() got fd %d\n\n", fd);
  if (fd < 0) {
    return fd;
  }


  printf("lseeking pos %d\n\n", pos);  

  r = lseek(fd, pos, SEEK_SET);
  if (r < 0) 
    return r;

  len = strlen(str);
  printf("writing test string at pos %d\n", pos);  
  
  i = 0;
  do {
    r = write(fd, &str[i], len-i);
    printf("wrote %d bytes\n", r);
    
    if (r < 0) {
      return r;
    }
    i += r;
  } while (i < len);
  

  printf("\nlseek to pos %d\n", pos);

  r = lseek(fd, pos, SEEK_SET);
  
  printf("\nreading %d bytes into buffer \n", len);
  i = 0;
  do  {  
    printf("attempting read of %d bytes\n", len -i);
    r = read(fd, &buf[i], len - i);
    printf("read %d bytes\n", r);
    i += r;
  } while (i < len && r > 0);
  
  printf("reading complete\n");
  if (r < 0) {
    return r;
  }
  i = 0;
  do {
    if (buf[i] != str[i]) {
      printf("ERROR  file contents mismatch\n");
      return -1;
    }
    i++;
  } while (i < len);
  printf("\nfile content okay\n");

  r = fstat(fd, &statbuf);

  if (r < 0) {
    return r;
  }
  
  printf("file stats: size: %d blocks: %d \n", 
	 statbuf.st_size,
	 statbuf.st_blocks);

  close(fd);
  return 0;
}

/*
 * Try to write to different locations within the file.
 *
 * Locations correspond to start of direct, single indirect, double
 * indirect, and triple indirect blocks.
 */

int
main(int argc, char * argv[])
{
  off_t pos;
  int r;
  (void) argc;
  (void) argv;

  printf("\n************************************************************\n* Asst3 Tester\n");
  

  pos = 0;
  printf("************************************************************\n");
  printf("* Testing first file system block (pos %d)\n", pos);
  printf("************************************************************\n");

  
  r = write_read_test("lhd0:testfile", teststr1, pos);

  if (r < 0) {
    printf("ERROR testing file: %s\n", strerror(errno));
    exit(1);
  }

  /* block size = 512
     ndirect = 15 */

  pos = 15 * 512;

  printf("************************************************************\n");
  printf("* Testing file system indirect block (pos %d)\n", pos);
  printf("************************************************************\n");

  r = write_read_test("lhd0:testfile", teststr2, pos);

  
  if (r < 0) {
    printf("ERROR testing file: %s\n", strerror(errno));
    exit(1);
  }

  /* block size = 512
     ndirect = 15
     db block per indirect = 128
  */

  pos = (15 + 128) * 512;

  printf("************************************************************\n");
  printf("* Testing file system double indirect block (pos %d)\n", pos);
  printf("************************************************************\n");

  r = write_read_test("lhd0:testfile", teststr1, pos);
  
  
  if (r < 0) {
    printf("ERROR testing file: %s\n", strerror(errno));
    exit(1);
  }

  pos = (15 + 128 + 128*128 -1) * 512;

  printf("************************************************************\n");
  printf("* Testing last double indirect block (pos %d)\n", pos);
  printf("************************************************************\n");

  r = write_read_test("lhd0:testfile", teststr2, pos);
  
  
  if (r < 0) {
    printf("ERROR testing file:%s\n", strerror(errno));
    exit(1);
  }

  pos = (15 + 128 + 128*128) * 512;

  printf("************************************************************\n");
  printf("* Testing triple indirect block (pos %d)\n", pos);
  printf("************************************************************\n");

  r = write_read_test("lhd0:testfile", teststr2, pos);
  
  
  if (r < 0) {
    printf("ERROR testing file:%s\n", strerror(errno));
    exit(1);
  }
 
  pos = (15 + 128 + 128*128*128 -1) * 512;

  printf("************************************************************\n");
  printf("* Testing last triple indirect block (pos %d)\n", pos);
  printf("************************************************************\n");

  r = write_read_test("lhd0:testfile", teststr2, pos);
  
  
  if (r < 0) {
    printf("ERROR testing file:%s\n", strerror(errno));
    exit(1);
  }
  
  pos = (15 + 128 + 128*128 + 128*128*128) * 512;
  
  printf("************************************************************\n");
  printf("* Testing file system beyond max file size (pos %d)\n", pos);
  printf("************************************************************\n");
  
  r = write_read_test("lhd0:testfile", teststr1, pos);
  
  
  if (r < 0) {
    printf("EXPECTED ERROR testing file:%s\n", strerror(errno));
  }
  else {
    printf("Hmmm, your filesystem supports bigger than expected files ????\n");
  }

  printf("************************************************************\n");
  printf("Sync()'ing files and finished!!!\n");
  printf("************************************************************\n");

  sync();

  return 0;
}



