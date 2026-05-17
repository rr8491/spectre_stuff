#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PAGESIZE 4096


#define IOCTL_CHECK_PTE _IOW('p', 1, unsigned long)

#define IOCTL_CLEAR_YOUNG _IOW('p', 2, unsigned long)

int main() {

    // open kernel device
    int fd = open("/dev/ptewalk", O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    // allocate one anonymous page
    void* page = mmap(NULL,
                      PAGESIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS,
                      -1,
                      0);

    if (page == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    printf("mapped page at %p\n", page);

    // touch page so kernel creates real PTE
    *(volatile char *)page = 1;

    // ask kernel module to inspect PTE
    if (ioctl(fd, IOCTL_CHECK_PTE, (unsigned long)page) < 0) {
        perror("ioctl");
        munmap(page, PAGESIZE);
        close(fd);
        return 1;
    }



        
   if (ioctl(fd, IOCTL_CLEAR_YOUNG, (unsigned long)page) < 0) {
    perror("ioctl CLEAR_YOUNG");
    munmap(page, PAGESIZE);
    close(fd);
    return 1;
}




if (ioctl(fd, IOCTL_CHECK_PTE, (unsigned long)page) < 0) {
    perror("ioctl CHECK_PTE");
    munmap(page, PAGESIZE);
    close(fd);
    return 1;
}


 volatile char x = *(volatile char *)page;
(void)x;


 if (ioctl(fd, IOCTL_CHECK_PTE, (unsigned long)page) < 0) {
    perror("ioctl CHECK_PTE");
    munmap(page, PAGESIZE);
    close(fd);
    return 1;
}
    munmap(page, PAGESIZE);
    close(fd);

    return 0;
}
