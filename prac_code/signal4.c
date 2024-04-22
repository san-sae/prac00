#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/time.h>
#include<sys/wait.h>

// WEXITSTATUS : OS마다 다른 프로세스의  종료 코드를 표준 규약으로 변환하는 함수
// 자식 프로세스가 이미 완료되어 종료 코드를 받아도, 부모 프로세스가 대기 작업을 실행함으로써 자식의 종료 코드를 얻을 수 있음
void sigchld_handler(int sig){
    pid_t child;
    int exitcode;
    child = wait(&exitcode);

    printf("> child process %d is terminated with exitcode %d\n", child, WEXITSTATUS(exitcode));
}

int main(void){
    signal(SIGCHLD, sigchld_handler);

    pid_t child;
    // fork()를 통해 PID가 변수 child에 할당
    // child의 값이 0이 아닌 값일 경우, 부모를 위한 블록으로 이동
    if(child = fork()){
        sleep(3);
        kill(child, SIGTERM);
        while(1);
    }
    // child의 값이 0일 경우, 자식 프로세스가 실행되도록 함
    // 기본적으로 자식 프로세스는 아무 실행도 하지 않고 무한 루프를 돌림
    else{
        while(1);
    }
}
