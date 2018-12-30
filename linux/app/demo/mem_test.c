#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
//#define INTEG_TIME (*(volatile unsigned short *)0x40000000)
#define INTEG_TIME_ADDR 0x40000000
int main() {
	// ------------------------ Mem Test-----------------------------------------------
	printf("Unsigned short size is %d bytes.\n", sizeof(unsigned short));
	int fd, n;
	unsigned char *map_base;
	unsigned char *buffer;

	buffer = (char *)malloc(0xff * sizeof(char));
	if (buffer == NULL) {
		printf("Fail to malloc buffer.\n");
		return 0;
	}	
	memset(buffer, 0 ,0xff);

    	fd = open("/dev/mem", O_RDWR | O_SYNC); //
    	if(fd == -1) {
      		printf("Error_open: \n");
    	}
    	map_base = mmap(NULL, 0xff, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x0); //
    	if(map_base == NULL) {
     	 	printf("Error2: \n");
		exit(1);
    	} else {
     		 printf("sucessful: \n");
   	 }
	   
 	strcpy(map_base, "Test mem map.\n");
	memset(buffer, 0, 0xff);
	strcpy(buffer, map_base);
	printf("Buffer = %s\n", buffer);
	
	munmap(map_base, 0xff);
	free(buffer);

    	close(fd);
	return 0;
}
