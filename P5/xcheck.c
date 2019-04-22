#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/mman.h>

#define stat xv6_stat  // avoid clash with host struct stat
#define dirent xv6_dirent  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#undef stat
#undef dirent

void print_inode(struct dinode dip) {
	printf("file type: %d,", dip.type);
	printf("file type: %d,", dip.nlink);
	printf("file type: %d,", dip.size);
	printf("file type: %d,", dip.addrs[0]);
}

int main(int argc, char *argv[]) {
	int fd;
	if (argc == 2)
		fd = open(argv[1], O_RDONLY);
    else {
    	fprintf(stderr, "%s", "Usage: xcheck <file_system_image>\n");
    	exit(1);
    }
    if (fd < 0) {
    	fprintf(stderr, "%s", "image not found.\n");
    	exit(1);
    }

    struct stat sbuf;
    
    fstat(fd, &sbuf);
    
    //printf("Image that i rwad is %ld in size\n", sbuf.st_size);

    void *img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (*((int*)img_ptr) == -1)
    {
    	fprintf(stderr, "%s", "ERROR: Mmap failed.\n");
    	exit(1);	
    }

    struct superblock *sb = (struct superblock *) (img_ptr + 2 * BSIZE);
    //printf("size %d nblocks %d ninodes %d\n", sb->size, sb->nblocks, sb->ninodes);

    struct dinode *dip = (struct dinode *) (img_ptr + 2 * BSIZE);
    
    uint data_block_addr = dip[1].addrs[0];
    struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + data_block_addr * BSIZE);
    if (entry[0].inum != 1 || entry[1].inum != 1) {
    	fprintf(stderr, "%s", "ERROR: root directory does not exist.\n");
    	exit(1);
    }
}
