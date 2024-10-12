#define _GNU_SOURCE
#include "lwp.h"
#include "rr.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>

struct scheduler rr_publish = {rr_init, rr_shutdown, rr_admit, rr_remove, rr_next, rr_qlen};

scheduler mainsched = &rr_publish;

long tid_count = 1;

tid_t lwp_create(lwpfun fun, void *arg) {

    if (tid_count == 1){
        mainsched->init();
    }

    struct rlimit rlim;
    unsigned long defaultStackSize = 8 * 1024 * 1024; // 8MB

    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
        printf("Current soft stack limit: %llu\n", rlim.rlim_cur);
        defaultStackSize = rlim.rlim_cur;
    
    } else {
        perror("getrlimit (soft)");
        printf("Using a default stack size of %lu bytes\n", defaultStackSize);
    }

    //printf("Comes here");

    //void* s = mmap(NULL, defaultStackSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    void* s = mmap(NULL, defaultStackSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);



    rfile threadregisters;

    swap_rfiles(NULL, &threadregisters);

    context* threadcontext = (context*)malloc(sizeof(context));

    threadcontext->tid = tid_count;
    threadcontext->stack = s;
    threadcontext->stacksize = defaultStackSize;
    threadcontext->state = threadregisters;
    threadcontext->status = 0;
    threadcontext->lib_one = NULL;
    threadcontext->lib_two = NULL;
    threadcontext->sched_one = NULL;
    threadcontext->sched_two = NULL;
    threadcontext->exited = 0;    
    
    threadcontext->state.fxsave = FPU_INIT;
    threadcontext->state.rdi = (unsigned long)fun;
    threadcontext->state.rsi = (unsigned long)arg;

    s = s + defaultStackSize/(sizeof(unsigned long));
    s-= defaultStackSize;
    memcpy(s, lwp_wrap, sizeof(void*));
    s-= defaultStackSize;
    threadcontext->state.rbp = (unsigned long)s;
    threadcontext->state.rsp = (unsigned long)s;

    tid_count++;
    return tid_count - 1;
}

void lwp_wrap (lwpfun fun, void*arg) {
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}

void lwp_exit(int status) {
}

tid_t lwp_gettid(void) {
    return (tid_t)0;
}

void lwp_yield(void) {
}

void lwp_start(void) {
    //make thread for the original context and admit 
}

tid_t lwp_wait(int *status) {
    return (tid_t)0;
}

void lwp_set_scheduler(scheduler sched) {

    if (sched == NULL) {
        sched = &rr_publish;
    }

    sched->init();

    // Transfer all threads from the old scheduler to the new one in next() order
    thread transferthread = mainsched->next();
    while (transferthread) {
        mainsched->remove(transferthread);
        sched->admit(transferthread);
        thread transferthread = mainsched->next();
    }

    //shuting down old scheduler
    mainsched->shutdown();

    //assigning the new scheduler to the mainsched variable
    mainsched = sched; 

}

scheduler lwp_get_scheduler(void) {
    return mainsched;
}

thread tid2thread(tid_t tid) {
    return NULL;
}


