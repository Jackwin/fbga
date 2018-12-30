#include <stdio.h>
#include <stdlib.h> 
#include <fcntl.h>

int main() {
	int valuefd, exportfd, directionfd; 
	printf("GPIO test running...\n");

	exportfd = open("/sys/class/gpio/export", O_WRONLY); 
	if (exportfd < 0) {
		 printf("Cannot open GPIO to export it\n"); 
		exit(1);
	 }
	 
	write(exportfd, "873", 32); // The width should be the GPIO_WIDTH export 873 -- LED
    	close(exportfd);
 
    	printf("GPIO exported successfully\n");
	
	// Set the direction	
	directionfd = open("/sys/class/gpio/gpio873/direction", O_RDWR); 
	if (directionfd < 0) { 
		printf("Cannot open GPIO direction it\n"); 
		exit(1);
	 }
	write(directionfd, "out", 32); 
	close(directionfd); 
	printf("GPIO direction set as output successfully\n");

	// Set the value
	valuefd = open("/sys/class/gpio/gpio873/value", O_RDWR); 
	if (valuefd < 0) { 
		printf("Cannot open GPIO value\n"); 
		exit(1); 
	}
	 printf("GPIO value opened, now toggling...\n");



	while (1) { 
		write(valuefd,"1", 2); 
		sleep(1); 
		write(valuefd,"0", 2);
		 sleep(1);
	 }

}
