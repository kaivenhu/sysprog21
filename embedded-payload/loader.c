/* A program that executes a second (embedded) ELF */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* No glibc wrappers exist for memfd_create(2), so provide our own. */
#include <sys/syscall.h>
static inline int memfd_create(const char *name, unsigned int flags)
{
    return syscall(__NR_memfd_create, name, flags);
}

/* ELF format
 * https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
 */
static bool valid_elf(char *ptr)
{
    return (ptr[4] == 1 || ptr[4] == 2) /* offset 0x4: 32/64-bit format */ &&
           (ptr[5] == 1 || ptr[5] == 2) /* offset 0x5: endianness */ &&
           (ptr[6] == 1); /* offset 0x6: current version */
}

int main(int argc, char *argv[], char **envp)
{
    int pid = getpid();
    int ret = 0;

    char proc_path[32];
    sprintf(proc_path, "/proc/%d/exe", pid);
    int filedesc = open(proc_path, O_RDONLY);
    if (filedesc < 0) {
        printf("Invalid file descriptor for /proc: %d\n", filedesc);
        return -1;
    }

    /* Find the size of this executable */
    struct stat st;
    stat(proc_path, &st);
    size_t size = st.st_size;

    char *entirefile = malloc(size);
    if (!entirefile) {
        printf("Insufficient memory.\n");
        return -2;
    }

    read(filedesc, entirefile, size);
    close(filedesc);

    /* find the second ELF header, which 52 or 64 bytes long for 32-bit and
     * 64-bit binaries respectively.
     */
    const char elf_magic[] = {0x7F, 'E', 'L', 'F'};
    char *newelf = memmem(entirefile + 52, size - 52, elf_magic, 4);
    if (newelf && !valid_elf(newelf)) /* forcely find again for real ELF */
        newelf = memmem(newelf + 6, size - (intptr_t) newelf - 6, elf_magic, 4);
    if (!newelf || !valid_elf(newelf)) {
        printf("No second ELF header found.\n");
        ret = -3;
        goto cleanup;
    }

    int newsize = size - (newelf - entirefile);
    int memfd = memfd_create("hidden", 0);
    if (memfd < 0) {
        printf("Invalid memfd.\n");
        ret = -4;
        goto cleanup;
    }

    /* Write ELF to temporary memory file */
    write(memfd, newelf, newsize);

    // Deploy the payload as a different process
    fork();
    if (getpid() != pid) {
        ret = fexecve(memfd, argv, envp); /* Execute the in-memory ELF */
        /* The above will only return if there is an error. */
        printf("Fail to execute payload. ret=%d (%s)\n", ret, strerror(errno));
    }

cleanup:
    free(entirefile);
    return ret;
}
