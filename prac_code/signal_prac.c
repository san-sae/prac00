#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<limits.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/wait.h>

void handler(int sig){
    printf("ring\n");
}

int main(void){
    struct itimerval t;

    signal(SIGALRM, handler);

    t.it_value.tv_sec = 1;
    t.it_value.tv_usec = 100000;
    t.it_interval = t.it_value;

    setitimer(ITIMER_REAL, &t, 0x0);

    while(1);
}
