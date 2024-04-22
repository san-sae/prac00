#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

// 본래 Ctrl + c를 누를 경우, 기본 동작은 프로그램을 즉시 종료하는 것
// 아래 함수를 통해 해당 동작을 재정의 가능
void handler(int sig){
    if(sig == SIGINT){
        printf("Do you want to quit?");
        if(getchar() == 'y'){
            exit(0);
        }
    }
}

int main(void){
    signal(SIGINT, handler);
    while(1);
}
