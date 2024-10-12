#include "schedulers.h"
#include "rr.h"
#include <stdlib.h>

static struct Node* rr_head = NULL;
static struct Node* rr_tail = NULL;
static struct Node* rr_current = NULL;

void rr_init() {
    rr_head = NULL;
    rr_tail = NULL;
    rr_current = NULL;
}

void rr_shutdown() {
}

void rr_admit(thread new) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        return;
    }
    newNode->data = new;
    newNode->next = NULL;
    if (rr_tail == NULL) {
        rr_head = newNode;
        rr_tail = newNode;
        rr_current = newNode;
    } else {
        rr_tail->next = newNode;
        rr_tail = newNode;
    }
}

void rr_remove(thread victim) {
    struct Node* current = rr_head;
    struct Node* prev = NULL;
    while (current != NULL) {
        if (current->data == victim) {
            if (prev == NULL) {
                rr_head = current->next;
            } else {
                prev->next = current->next;
            }
            if (current == rr_current) {
                rr_current = prev; 
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

thread rr_next() {
    if (rr_head != NULL) {
        if (rr_current == NULL || rr_current->next == NULL) {
            rr_current = rr_head; // If current is at the end, loop to the beginning
        } else {
            rr_current = rr_current->next;
        }
        return rr_current->data;
    }
    return NULL;
}

int rr_qlen() {
    int count = 0;
    struct Node* current = rr_head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}
