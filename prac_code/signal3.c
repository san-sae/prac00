#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

/*
kill signal을 보낼 수 있는 이유는 내가 만든 프로세스이면서 같은 사용자이기 때문(다른 프로세스에 대한 권한 없음)
부모 프로세스는 각 자식 프로세스에 대한 관리 권한 가짐
sudo : 다른 프로세스에 대한 관리 권한 있음
*/
/*
두 개의 터미널 이용
하나는 signal3.c 파일을 실행하고, 다른 하나는 kill 명령어를 이용해 종료시킴
ps 명령어로 a.out 파일의 PID 찾고, kill <a.out의 PID>로 종료
*/
void handler(int sig){
    printf("Do you want to quit?");
    if(getchar() == 'y'){
        exit(0);
    }
}

int main(){
    int d;
    signal(SIGTERM, handler);
    scanf("%d", &d);
}
