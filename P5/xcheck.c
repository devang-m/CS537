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
	printf("nlink: %d,", dip.nlink);
	printf("size: %d,", dip.size);
	printf("address: %d,", dip.addrs[0]);
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
    
    void *img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (*((int*)img_ptr) == -1)
    {
    	fprintf(stderr, "%s", "ERROR: Mmap failed.\n");
    	exit(1);	
    }

    struct superblock *sb = (struct superblock *) (img_ptr + BSIZE);
    //printf("size %d nblocks %d ninodes %d\n", sb->size, sb->nblocks, sb->ninodes);

    int blocks_in_use[sb->size];
    memset(blocks_in_use, 0, (sb->size)*sizeof(blocks_in_use[0]));

    struct dinode *dip = (struct dinode *) (img_ptr + 2 * BSIZE);

    for (int i=0; i < sb->ninodes; i++) {
    	if (dip[i].type < 0 || dip[i].type > 3){
    	    fprintf(stderr, "%s", "ERROR: bad inode.\n");
    	    exit(1);
    	}
    }

    // Do we need to round up?
    void* bitmap = (void *) (img_ptr + 3*BSIZE + ((sb->ninodes/IPB)*BSIZE)); 
    void* data_block_start = bitmap + (((sb->nblocks)/(BSIZE*8)));
    if((sb->nblocks)%(BSIZE*8) > 0) {
    	data_block_start = data_block_start + BSIZE;
    }
    uint data_block_start_number = (data_block_start-img_ptr)/BSIZE;
    for (int i=0; i < sb->ninodes; i++) {
    	if (dip[i].type > 0 && dip[i].type <= 3) {
    		// Direct check
    	    for (int x=0; x < NDIRECT; x++) {
    	    	if(dip[i].addrs[x]!=0 && (dip[i].addrs[x] < data_block_start_number || dip[i].addrs[x] >= data_block_start_number + sb->nblocks)) {
    	    		fprintf(stderr, "%s", "ERROR: bad direct address in inode.\n");
    	    	 	exit(1);
    	    	}
    	    	if((dip[i].addrs[x]!=0) && blocks_in_use[dip[i].addrs[x]] == 1) {
    	    		fprintf(stderr, "%s", "ERROR: direct address used more than once.\n");
    	    	 	exit(1);
    	    	}
    	    	blocks_in_use[dip[i].addrs[x]] = 1;
    	    }
    	    
    	    // Indirect check
	    	if(dip[i].addrs[NDIRECT]!=0 && (dip[i].addrs[NDIRECT] < data_block_start_number || dip[i].addrs[NDIRECT] >= data_block_start_number + sb->nblocks)) {
	    		fprintf(stderr, "%s", "ERROR: bad indirect address in inode.\n");
	    	 	exit(1);
	    	}
	    	if((dip[i].addrs[NDIRECT]!=0) && blocks_in_use[dip[i].addrs[NDIRECT]] == 1) {
	    		fprintf(stderr, "%s", "ERROR: indirect address used more than once.\n");
	    	 	exit(1);
    	    }
	    	blocks_in_use[dip[i].addrs[NDIRECT]] = 1;
	    	uint* indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
    	    for (int x = 0; x < NINDIRECT; x++) {
    	    	if(indirect[x]!=0 && (indirect[x] < data_block_start_number || indirect[x] >= data_block_start_number + sb->nblocks)) {
    	    		fprintf(stderr, "%s", "ERROR: bad indirect address in inode.\n");
    	    	 	exit(1);
    	    	}
    	    	if((indirect[x]!=0) && blocks_in_use[indirect[x]] == 1) {
    	    		fprintf(stderr, "%s", "ERROR: indirect address used more than once.\n");
    	    	 	exit(1);
    	    	}
    	    	blocks_in_use[indirect[x]] = 1;
    	    }
    	}
    }	

    //print_inode(dip[0]);

    // Do we need to check for T_DEV as well?
    // Checking if dir is properly formatted
    for (int i=0; i < sb->ninodes; i++) {
    	if (dip[i].type == 1) {
    		struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + dip[i].addrs[0]*BSIZE);
    		if (strcmp(entry[1].name, "..") != 0 || strcmp(entry[0].name, ".") != 0 || entry[0].inum != i) {
    			fprintf(stderr, "%s", "ERROR: directory not properly formatted.\n");
    			exit(1);
    		}
    	}
    }

    //Ensure that proper formatting of directory is checked before root directory check.
    //Check for root directory, are . and .. at first two entries in the directory and root in 1
    uint data_block_addr = dip[1].addrs[0];
    struct xv6_dirent *entry = (struct xv6_dirent *)(img_ptr + data_block_addr * BSIZE);
    if (entry[0].inum != 1 || entry[1].inum != 1) {
    	fprintf(stderr, "%s", "ERROR: root directory does not exist.\n");
    	exit(1);
    }

    // Do we need to round up
    char* bitmap_start = (char *) (img_ptr + 3*BSIZE + ((sb->ninodes/IPB)*BSIZE));
    //printf("%ud %ud\n", &(*bitmap_start),  &(*img_ptr));
    for (int i = 0; i < sb->ninodes; i++) {
    	if (dip[i].type > 0 && dip[i].type <= 3){
    		for(int j = 0; j < NDIRECT+1; j++) {
    			if(dip[i].addrs[j] == 0) {
    				continue;
    			}
    			uint bitmap_array_pos = (dip[i].addrs[j])/8;
    			uint bit_position_within_byte = (dip[i].addrs[j])%8;
    			if(((bitmap_start[bitmap_array_pos] >> bit_position_within_byte) & 1) == 0) {
    				//printf("%d %d\n", i. j);
    				fprintf(stderr, "%s", "ERROR: address used by inode but marked free in bitmap.\n");
    	    		exit(1);
    			}
    		}
    		uint* indirect = (uint*) (img_ptr + BSIZE * dip[i].addrs[NDIRECT]);
    	    for (int x = 0; x < NINDIRECT; x++) {
    	    	//uint bit = BBLOCK(dip[i].addrs[x], sb->inodes);
    	    	uint indirect_addr = indirect[x];
    	    	if(indirect_addr == 0) {
    				continue;
    			}
    			uint bitmap_array_pos = (indirect_addr)/8;
    			uint bit_position_within_byte = (indirect_addr)%8;
    			if(((bitmap_start[bitmap_array_pos] >> bit_position_within_byte) & 1) == 0) {
    				fprintf(stderr, "%s", "ERROR: address used by inode but marked free in bitmap.\n");
    	    		exit(1);
    			}
    	    	//char* bitmap_block_address = (char*)(bitmap_block * BSIZE + img_ptr); 
    	    }
    	}
    }

    for (int i=0; i<sb->nblocks; i++) {
    	uint bitmap_array_pos = (i+data_block_start_number)/8;
    	uint bit_position_within_byte = (i+data_block_start_number)%8;
    	//int bound = sb->size
    	if(((bitmap_start[bitmap_array_pos] >> bit_position_within_byte) & 1) == 1) {
    		if(blocks_in_use[i+data_block_start_number]!=1) {
    			fprintf(stderr, "%s", "ERROR: bitmap marks block in use but it is not in use.\n");
    	    	exit(1);
    		}
    	}
    }


}

//ndirect+1 is indirect block - not sure if we need to check if the indirecr addresses within it are valid or not
