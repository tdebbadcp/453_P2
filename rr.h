#ifndef RR_H
#define RR_H

#include "lwp.h"

extern struct scheduler rr_publish;

extern void rr_init(void);
extern void rr_shutdown(void);
extern void rr_admit(thread new);
extern void rr_remove(thread victim);
extern thread rr_next(void);
extern int rr_qlen(void);

struct Node {
    thread data;
    struct Node* next;
};

#endif // RR_H
