#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

void fpsavetest_save(int *);
void fpsavetest_load(const int *);

static _Atomic int running;

static const int nregs = 16;

static void
clobber(int x)
{
        int fp[nregs];
        int i;
        for (i = 0; i < nregs; i++) {
                fp[i] = x++;
        }
        fpsavetest_load(fp);
}

static void
sighandler(int signo)
{
        clobber(0);

        /* note: this is not safe because printf is not signal safe */
        printf("%s signo=%d\n", __func__, signo);
        alarm(2);
}

static void *
thread(void *vp)
{
        int x = 0;
        while (running) {
                printf("clobber loop x=%x\n", x);
                clobber(x++);
                sleep(1);
        }
        return NULL;
}

static void
fp_check_loop(void)
{
        int fp[nregs];
        int x = 0xabcdef;
        int i;
        for (i = 0; i < nregs; i++) {
                fp[i] = x--;
        }
        fpsavetest_load(fp);
        while (running) {
                int cfp[nregs];
                fpsavetest_save(cfp);
                for (i = 0; i < nregs; i++) {
                        if (fp[i] != cfp[i]) {
                                fprintf(stderr,
                                        "corruption detected fp%d expected "
                                        "0x%08x actual 0x%08x\n",
                                        i, fp[i], cfp[i]);
                        }
                }
        }
}

int
main(int argc, char **argv)
{
        pthread_t t;
        int ret;
        running = 1;
        if (signal(SIGALRM, sighandler) == SIG_ERR) {
                fprintf(stderr, "signal failed with %d\n", errno);
                exit(1);
        }
        alarm(2);
        ret = pthread_create(&t, NULL, thread, NULL);
        if (ret != 0) {
                fprintf(stderr, "pthread_create failed with %d\n", ret);
                exit(1);
        }
        fp_check_loop();
        void *v;
        ret = pthread_join(t, &v);
        if (ret != 0) {
                fprintf(stderr, "pthread_join failed with %d\n", ret);
        }
        exit(1);
}
