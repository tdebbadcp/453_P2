#define _GNU_SOURCE
#include "lwp.h"
#include "rr.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>

//struct scheduler rr_publish = {rr_init, rr_shutdown, rr_admit, rr_remove, rr_next, rr_qlen};
struct scheduler rr_publish;
scheduler mainsched = &rr_publish;

//Global linked list of threads
thread headthread = NULL;

//Global linked list of exited threads
thread headexitedthread = NULL;

long tid_count = 1;

tid_t lwp_create(lwpfun fun, void *arg) {

    // if (tid_count == 1){
    //     mainsched->init();
    // }

    printf("Before getrlimit\n");

    struct rlimit rlim;
    unsigned long defaultStackSize = 8 * 1024 * 1024; // 8MB

    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
        printf("Current soft stack limit: %llu\n", rlim.rlim_cur);
        defaultStackSize = rlim.rlim_cur;
    
    } else {
        perror("getrlimit (soft)");
        printf("Using a default stack size of %lu bytes\n", defaultStackSize);
    }

    printf("before mmap\n");

    //void* s = mmap(NULL, defaultStackSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
    void* s = mmap(NULL, defaultStackSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    printf("After mmap\n");

    rfile threadregisters;

    //swap_rfiles(NULL, &threadregisters);

    context* threadcontext = (context*)malloc(sizeof(context));

    threadcontext->tid = tid_count;
    threadcontext->stack = s;
    threadcontext->stacksize = defaultStackSize;
    threadcontext->state = threadregisters;
    threadcontext->status = 0;
    threadcontext->lib_one = NULL; //Using to point to the next thread in the global linked list
    threadcontext->lib_two = NULL;
    threadcontext->sched_one = NULL;
    threadcontext->sched_two = NULL;
    threadcontext->exited = 0;    
    
    threadcontext->state.fxsave = FPU_INIT;
    threadcontext->state.rdi = (unsigned long)fun;
    threadcontext->state.rsi = (unsigned long)arg;

    s = s + defaultStackSize/(sizeof(unsigned long));
    s-= defaultStackSize;
    //memcpy(s, lwp_wrap, sizeof(void*));
    s-= defaultStackSize;
    threadcontext->state.rbp = (unsigned long)s;
    threadcontext->state.rsp = (unsigned long)s;

    //Admitting thread to scheduler
    mainsched->admit(threadcontext);
    printf("Admitted thread to scheduler\n");

    //Making thread the head of the global linked list
    if (headthread){
        threadcontext->lib_one = headthread;  
    }
    headthread = threadcontext;  
    printf("Added thread to global linked list\n"); 

    tid_count++;
    return tid_count - 1;
}

void lwp_wrap(lwpfun fun, void*arg) {
    int rval;
    rval = fun(arg);
    lwp_exit(rval);
}

void lwp_exit(int status) {
    //take it out of roundrobin
    //keep a pointer to the exited thread, use the pointer provided in the context struct
    //head -> firstexited -> secondexited -> thirdexited -> NULL
    //if head is null, no one has exited
    //If head is not null, deallocate what head points to, then make head point to the next exited thread
}

tid_t lwp_gettid(void) {
    return (tid_t)0;
}

void lwp_yield(void) {
    thread nextthread = mainsched->next();
    if (nextthread){
        swap_rfiles(NULL , &nextthread ->state);
    }
    // else{
    //     lwp_exit() //Need to work on this
    // }
}

void lwp_start(void) {
    //make thread for the original context(main) and admit 

    struct rlimit rlim;
    unsigned long defaultStackSize = 8 * 1024 * 1024; // 8MB

    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
        printf("Current soft stack limit: %llu\n", rlim.rlim_cur);
        defaultStackSize = rlim.rlim_cur;
    
    } else {
        perror("getrlimit (soft)");
        printf("Using a default stack size of %lu bytes\n", defaultStackSize);
    }

    rfile mainthreadregisters;
    swap_rfiles(&mainthreadregisters, NULL);

    context* maincontext = (context*)malloc(sizeof(context));

    maincontext->tid = tid_count;
    maincontext->stack = NULL; // Main thread already has a stack, marking it as NULL
    maincontext->stacksize = defaultStackSize;
    maincontext->state = mainthreadregisters;
    maincontext->status = 0;
    maincontext->lib_one = NULL; //Using to point to the next thread in the global linked list
    maincontext->lib_two = NULL;
    maincontext->sched_one = NULL;
    maincontext->sched_two = NULL;
    maincontext->exited = 0;    

    //Admitting main thread to scheduler
    mainsched->admit(maincontext);
    printf("Admitted main thread to scheduler\n");

    //Making thread the head of the global linked list
    if (headthread){
        maincontext->lib_one = headthread;  
    }
    headthread = maincontext;  
    printf("Added main thread to global linked list\n"); 

    tid_count++;

}

tid_t lwp_wait(int *status) {
    //See if any thread has exited

    //how to keep track of exited threads? 
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


