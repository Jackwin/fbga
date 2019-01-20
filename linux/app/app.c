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
#include <sys/time.h>
#include <poll.h>

#include "gpio.h"
#include "cdma.h"
#include "network.h"
// ------------------------- GPIO -----------------------------------
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (0 * 1000) /* 3 seconds */
#define MAX_BUF 64

int G11620Start(unsigned int gpio_num);
int G11620SetIntegTime(int microsecond);

int G11620SetCapTime(int cap_time);
int main() {

    unsigned int out_gpio;
    int status;
    out_gpio = 873;
//    gpio_export(out_gpio);
  //  gpio_set_dir(out_gpio, 1);

// --------------------------------- gpio_in------------------------------------
    int g11620_done_fd;
    unsigned int g11620_done_gpio;

    // Set GPIO input
    g11620_done_gpio = 905;

    gpio_export(g11620_done_gpio);
    gpio_set_dir(g11620_done_gpio, 0);
    gpio_set_edge(g11620_done_gpio, "rising");
    g11620_done_fd = gpio_fd_open(g11620_done_gpio);

//Calculate the integration time
   unsigned int integ_time_ms = 300;
   status = G11620SetIntegTime(integ_time_ms);
   if (status == 0) {
	    printf("Fail to set the G11620 integration time.\n");
        return 0;
    }
    /* Set DMA */
    void *mapped_dev_base;
    mapped_dev_base =  SetCDMA();
    struct pollfd fds[1];
    memset((void*)fds, 0, sizeof(fds));

    char rd_data;
    unsigned long char_cnt;

    //char_cnt = BUFFER_BYTESIZE;
    printf("Ready.\n");

    /*Start G11620*/
    unsigned int g11620_start_gpio = 877;
    gpio_export(g11620_start_gpio);
    gpio_set_dir(g11620_start_gpio,1);
    //status = G11620Start(g11620_start_gpio);
    if (status != 1) {
        printf("Fail to start G11620.\n");
         return 0;
    }
    int cap_times = 0;
    while (1) {
        /*
        if (cap_times < 10) {
            status = G11620Start(g11620_start_gpio);
            if (status != 1) {
                printf("Fail to start G11620.\n");
                return 0;
            }
        }
         else {
            gpio_fd_close(g11620_done_fd);
            return 0;
        }
        cap_times++;
        printf("Capture is %d.\n", cap_times);
        */
        cap_times = 10;
        char_cnt = BUFFER_BYTESIZE * cap_times;
        G11620SetCapTime(cap_times);
        status = G11620Start(g11620_start_gpio);
        if (status != 1) {
            printf("Fail to start G11620.\n");
            return 0;
        }
        while(1){

//            fds[0].fd = g11620_done_fd;
//            fds[0].events = POLLPRI;
            lseek(g11620_done_fd, 0, SEEK_SET);
            read(g11620_done_fd, &rd_data, sizeof(rd_data)); // Initial read to prevent false edge
/*            if (poll(fds, 1, POLL_TIMEOUT) == -1) {
                perror(" G11620_done_fd poll failed!\n");
                return -1;
             }
             */
//            printf("rd_data is %x.\n", rd_data);
            if (rd_data == '1') {
                printf("Capture the edge.\n");
                status = DMAStart(mapped_dev_base, char_cnt);
                int* mapped_ddr_base = MapDDR2UserSpace();
                EthernetClient(mapped_ddr_base);
                UnmapDDR2UserSpace(mapped_ddr_base);
//                fflush(stdout);
                break;
            }
/*
            if (fds[0].revents & POLLPRI) {
                if (lseek(g11620_done_fd, 0, SEEK_SET) == -1) {
                    perror("lseek failed!\n");
                    return -1;
                }
                int len;
                if ((len = read(g11620_done_fd, &rd_data, sizeof(rd_data))) == -1) {
                    perror("read failed!\n");
                    return -1;
                 }

                printf("Capture the edge.\n");
                status = DMAStart(mapped_dev_base, char_cnt);
                int* mapped_ddr_base = MapDDR2UserSpace();
//                int i;
//                for (i = 0; i < BUFFER_BYTESIZE / 4; i++) {
//                    printf("Offset is %d, and data is %x.\n", i, *(mapped_ddr_base + i)) ;
//                }
                EthernetClient(mapped_ddr_base);
                UnmapDDR2UserSpace(mapped_ddr_base);
                fflush(stdout);
                break;
            }*/
        }
    }
    gpio_fd_close(g11620_done_fd);

}

int G11620SetIntegTime(int microsecond) {

	if (microsecond < 3 || microsecond > 65535) {
		printf("Invalid integration time.\n");
		return 0;
	}

	int clk_cycle_cnt = microsecond / 0.2; // The clock cycle is 200 ns

    // The address of capture time register in PL is 0x4
    SetPLParamRAM(1, clk_cycle_cnt);
	return 1;
}

int G11620SetCapTime(int cap_time) {
    if (cap_time <= 0) {
        printf("Cap time should be larger than 0.\n");
        return 0;
    }
    // The address of capture time register in PL is 0x8
	SetPLParamRAM(2, cap_time);
    return 1;
}

int G11620Start(unsigned int gpio_num) {
    int status;
    status = gpio_set_value(gpio_num,0);
    if (status != 1) {
        printf("Fail to set value at GPIO%.\n", gpio_num);
        return 0;
    }
    gpio_set_value(gpio_num,1);
    if (status != 1) {
        printf("Fail to set value at GPIO%.\n", gpio_num);
        return 0;
    }
    gpio_set_value(gpio_num,0);
    if (status != 1) {
        printf("Fail to set value at GPIO%.\n", gpio_num);
        return 0;
    }
    return 1;

}
