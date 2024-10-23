#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"
#include "schedulers.h"

#define MAXSNAKES  100

static void testfun(void *num);

int main(int argc, char *argv[]){
  
  long i;

  printf("Launching LWP\n");
  /* spawn a number of individual LWPs */
  lwp_create((lwpfun)testfun,(void*)1);

  lwp_start();
  
}

static void testfun(void *num) {
  int newnum;
  newnum=(long)num;     
  printf("Hello %d\n", newnum);

}
