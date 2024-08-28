#undef NDEBUG
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "esp32_spiram.h"

#define SOC_EXTRAM_DATA_LOW 0x3F800000
#define MMU_PAGE_SIZE_IN_KB (32)
#define MMU_PAGE_SIZE (MMU_PAGE_SIZE_IN_KB * 1024)

void
migrate_to(int cpuid)
{
        int ret;
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(cpuid, &mask);
        ret = sched_setaffinity(gettid(), sizeof(mask), &mask);
        assert(ret == 0);
        while (cpuid != sched_getcpu()) {
                sleep(1);
        }
}

void
loop(int id)
{
        migrate_to(id);

        int hwcpuid = up_cpu_index();
        int ret;
        uint32_t va = SOC_EXTRAM_DATA_LOW;
        uint32_t pa = id * MMU_PAGE_SIZE;
        printf("%s tid=%d id=%d hwcpuid=%u va=%" PRIx32 " pa=%" PRIx32 "\n",
               __func__, (int)gettid(), id, hwcpuid, va, pa);

        ret = cache_sram_mmu_set(hwcpuid, 0, va, pa, MMU_PAGE_SIZE_IN_KB, 1);
        assert(ret == 0);

        snprintf((void *)va, MMU_PAGE_SIZE,
                 "this is data for cpu %d, at pa %" PRIx32, id, pa);

        for (;;) {
                printf("%s id=%d curcpu=%d hwcpuid=%d data \"%s\"\n", __func__,
                       id, sched_getcpu(), up_cpu_index(), (const char *)va);
                sleep(1);
        }
}

void *
thread(void *vp)
{
        loop((int)vp);
        return NULL;
}

int
main(void)
{
        int ret;
        pthread_t t;
        ret = pthread_create(&t, NULL, thread, (void *)0);
        assert(ret == 0);
        sleep(3);
        loop(1);
        return 0;
}
