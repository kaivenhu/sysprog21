#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define PAGE_SHIFT 12

#define MYLOG "/dev/mylog"

int main(int argc, char *argv[])
{
    int fd;
    void *map_memory = NULL;
    int idx;

    if (3 > argc || NULL == argv[1] || NULL == argv[2]) {
        printf("Usage: user <index> <context>\n");
        exit(1);
    }

    idx = atoi(argv[1]);

    errno = 0;
    fd = open(MYLOG, O_RDWR);
    if (-1 == fd) {
        printf("Failed to open %s, error: %d\n", MYLOG, errno);
        exit(1);
    }

    map_memory = mmap(NULL, 32, PROT_WRITE, MAP_SHARED, fd, idx << PAGE_SHIFT);
    if (MAP_FAILED == map_memory) {
        printf("Failed to mmap, error: %d\n", errno);
        exit(1);
    }

    sprintf((char *) map_memory, "%s", argv[2]);

    munmap(map_memory, 32);

    close(fd);

    return 0;
}
