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

// ------------------------- GPIO -----------------------------------
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

int main() {
    unsigned int gpio;
    unsigned int status;
    gpio = 873;
    gpio_export(gpio);
    gpio_set_dir(gpio, 1);

// GPIO input
    int g11620_done_fd;
    gpio = 905;
    gpio_export(gpio);
    gpio_set_dir(gpio, 0);
    gpio_set_edge(gpio, "rising");
    g11620_done_fd = gpio_fd_open(gpio);

    SetPLParamRAM(1, 0xfff0);

    void *mapped_dev_base;
    mapped_dev_base =  SetCDMA();
    struct pollfd fds[1];
    memset((void*)fds, 0, sizeof(fds));

    char rd_data;
    unsigned long char_cnt;
    char_cnt = BUFFER_BYTESIZE;

    while (1) {
        fds[0].fd = g11620_done_fd;
        fds[0].events = POLLPRI;
        read(g11620_done_fd, &rd_data, sizeof(rd_data)); // Initial read to prevent false edge
        if (poll(fds, 1, POLL_TIMEOUT) == -1) {
            perror(" G11620_done_fd poll failed!\n");
            return -1;
        }

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
            if (rd_data == 0x31) {
                status = DMAStart(mapped_dev_base, char_cnt);
            }
        }
        fflush(stdout);
    }
    gpio_fd_close(g11620_done_fd);

}


