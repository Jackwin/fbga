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
//#define INTEG_TIME (*(volatile unsigned short *)0x40000000)i
#define PL_PARAM_RAM_ADDR 0x40000000
#define INTEG_TIME_ADDR (PL_PARAM_RAM_ADDR + 4)
#define SIZE 0x20
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
    	map_base = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PL_PARAM_RAM_ADDR); //
    	if(map_base == NULL) {
     	 	printf("Error2: \n");
		exit(1);
    	} else {
     		 printf("sucessful: \n");
   	 }
	   
// 	strcpy(map_base, "Test mem map.\n");
//	memset(buffer, 0, 0xff);
//	strcpy(buffer, map_base);
//	printf("Buffer = %s\n", buffer);
	
	int i; 	
	for (i = 0; i < SIZE; i++) {
		map_base[i] = i;
	}
	//map_base[0] = 0xdd;
	for (i = 0; i < SIZE; i++)	
		printf("map_base[%d] is %x.\n", i, map_base[i]);
	munmap(map_base, SIZE);
	free(buffer);

    	close(fd);
	return 0;
	 
}
