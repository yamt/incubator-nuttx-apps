#undef NDEBUG
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

#include "esp32_spiram.h"

#define SOC_EXTRAM_DATA_LOW 0x3F800000
#define MMU_PAGE_SIZE_IN_KB (32)
#define MMU_PAGE_SIZE (MMU_PAGE_SIZE_IN_KB * 1024)

#if 0
static void
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
#endif

void loop(int id) {
  // migrate_to(id);
  int pid;
  switch (id) {
  case 1:
    pid = 3;
    break;
  default:
    pid = 5;
    break;
  }

  int hwcpuid = up_cpu_index();
  uint32_t va = SOC_EXTRAM_DATA_LOW;
  uint32_t pa = pid * MMU_PAGE_SIZE;
  printf("%s pid=%d tid=%d id=%d hwcpuid=%u\n", __func__, pid, (int)gettid(),
         id, hwcpuid);

  int ret;
  printf("%s calling SYS_sram_mmu_set\n", __func__);
  ret = sys_call4(SYS_sram_mmu_set, pid, SOC_EXTRAM_DATA_LOW, pa, 1);
  printf("%s SYS_sram_mmu_set returned ret=%d\n", __func__, ret);
  printf("%s calling SYS_set_pid\n", __func__);
  sys_call1(SYS_set_pid, pid);
  printf("%s returned from SYS_set_pid\n", __func__);

  snprintf((void *)va, MMU_PAGE_SIZE, "this is data for pid %d, at pa %" PRIx32,
           pid, pa);

  for (;;) {
    printf("%s pid=%d id=%d curcpu=%d hwcpuid=%d data \"%s\"\n", __func__, pid,
           id, sched_getcpu(), up_cpu_index(), (const char *)va);
    sleep(1);
  }
}

void *thread(void *vp) {
  loop((int)vp);
  return NULL;
}

int main(void) {
  int ret;
  pthread_t t;
  ret = pthread_create(&t, NULL, thread, (void *)0);
  assert(ret == 0);
  sleep(3);
  loop(1);
  return 0;
}
