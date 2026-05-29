#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <x86intrin.h>

#define PAGESIZE 4096


#define IOCTL_CHECK_PTE _IOW('p', 1, unsigned long)

#define IOCTL_CLEAR_YOUNG _IOW('p', 2, unsigned long)


static inline uint64_t time_access(volatile char *addr)
{
    unsigned int aux;
    uint64_t start, end;
    volatile char value;

    _mm_lfence();
    start = __rdtscp(&aux);

    value = *addr;
    (void)value;

    _mm_lfence();
    end = __rdtscp(&aux);

    return end - start;
}

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



    // clear accessed bit
   if (ioctl(fd, IOCTL_CLEAR_YOUNG, (unsigned long)page) < 0) {
    perror("ioctl CLEAR_YOUNG");
    munmap(page, PAGESIZE);
    close(fd);
    return 1;
}



// check if accessed bit is 0
if (ioctl(fd, IOCTL_CHECK_PTE, (unsigned long)page) < 0) {
    perror("ioctl CHECK_PTE");
    munmap(page, PAGESIZE);
    close(fd);
    return 1;
}

    // access mapped page so it is cached
    volatile char warm = *(volatile char *)page;
    (void)warm;

    // measure cached access time
    uint64_t cached_time = time_access((volatile char *)page);
    printf("cached access time: %lu cycles\n", cached_time);

    // flush mapped page from cache
    _mm_clflush(page);
    _mm_mfence();

    // measure flushed access time
    // this access brings it back into cache
    uint64_t flushed_time = time_access((volatile char *)page);
    printf("flushed access time: %lu cycles\n", flushed_time);

    // flush again so speculative gadget starts with page uncached
    _mm_clflush(page);
    _mm_mfence();


for (int i = 0; i < 100000; i++) {

    asm volatile(
        "call 1f\n\t"  // pushes return address onto stack

        "movb (%[target]), %%al\n\t" // 1f should return to here once it returns

        "jmp 2f\n\t"

        "1:\n\t"
        "pop %%rax\n\t" // removes movb inst from architectural stack, RSB still has the movb instr
        "lea 2f(%%rip), %%rax\n\t" // put address of 2f in rax
        "push %%rax\n\t" // push rax to architectural stack
        "clflush (%%rsp)\n\t"
        "mfence\n\t"
        "ret\n\t" // this goes to address of label2, but RSB should predict movb instr

        "2:\n\t"
        :
        : [target] "r" (page)
        : "rax", "memory"
    );
} 

sched_yield();


    // measure after speculative gadget
    uint64_t post_spec_time = time_access((volatile char *)page);
    printf("post-spec access time: %lu cycles\n", post_spec_time);

    if (post_spec_time < flushed_time && post_spec_time <= cached_time * 2) {
        printf("likely speculative cache hit\n");
    } else {
        printf("likely no speculative cache hit\n");
    }

    // check whether speculative access set bit
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


//logic
/*
1. mmap page
2. touch page normally, this will set accessed bit to 1 (32)
3. clear accessed bit, will set to 0
4. check cleared state, print accessed bit 0
5. run RSB speculative block
6. check accessed bit again

*/