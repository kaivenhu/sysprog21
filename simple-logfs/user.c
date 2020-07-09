#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MYLOG "/dev/mylog"

int main(void)
{
    char buf[32] = {'\0'};

    int fd;
    void *map_memory = NULL;

    errno = 0;
    fd = open(MYLOG, O_RDWR);
    if (-1 == fd) {
        printf("Failed to open %s, error: %d\n", MYLOG, errno);
        exit(1);
    }

    map_memory = mmap(NULL, 256, PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == map_memory) {
        printf("Failed to mmap, error: %d\n", errno);
        exit(1);
    }

    while (0 < scanf("%s", buf)) {
        if (0 == strcmp(buf, "exit")) {
            break;
        }
        sprintf((char *) map_memory, "%s", buf);
        memset(buf, 0, sizeof(buf));
    }

    munmap(map_memory, 256);

    close(fd);

    return 0;
}
