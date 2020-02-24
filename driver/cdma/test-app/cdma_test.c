#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
int main(int argc, char **argv)
{
	int i;
	int fd;
	int val=1;
	fd = open("/dev/gpio_drv", O_RDWR); //打开设备
	if (fd <= 0) {
		Printf(“open %s error!\n” , filename)
		return -1;
	}
	//write operation
	while(1)
	{
		for(i=0;i<4;i++)
		{
			val = (1<<i);
			write(fd,&val,4); //write val to fd device, size is 4 byte
			sleep(1);
		}
	}
	return 0;
}