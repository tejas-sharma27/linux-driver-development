#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define MAGIC 'V'
#define CLEAR_BUFFER 1
#define GET_LENGTH 2
#define SET_LENGTH _IOW(MAGIC, 3, int)
int main() {
    int fd = open("/dev/hello_driver", O_RDWR);
    ssize_t len = 0;
    ioctl(fd, GET_LENGTH, &len);
    printf("Length of the data is %ld.\n", len);
    ioctl(fd, CLEAR_BUFFER);
    printf("Data cleared from the device.\n");
    ioctl(fd, GET_LENGTH, &len);
    printf("Length of the data is %ld.\n", len);
    int user_data = 69;
    ioctl(fd, SET_LENGTH, &user_data);
    char buffer[256] = {0};
    ssize_t ret = read(fd, buffer, sizeof(buffer)-1);
    printf("Read is called.\n");
    sleep(10);
    ret = read(fd, buffer, sizeof(buffer)-1);
    printf("Read is called.\n");
    sleep(10);
    ret = read(fd, buffer, sizeof(buffer)-1);
    printf("Read is called.\n");
    close(fd);
    return 0;
}