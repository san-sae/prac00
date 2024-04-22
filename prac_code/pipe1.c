#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<error.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

// pipe : 두 프로세스 간의 통신을 위한 메커니즘 중 하나
//        OS에서 제공하는 기능으로, 두 프로세스 간 단방향 통신 채널 설정
// unnamed pipe : 동일한 파이프가 여러 자식 프로세스에 상속될 수 있음
//                but, 여러 프로세스 간의 통신에는 권장X

// 파일 디스크립터는 파일에 액세스하는 데 사용되는 일종의 키로 작동하는 정수 값
// 두 개의 파일 스크립터; 하나는 읽기용, 다른 하나는쓰기용
// buffer pointer를 받아 파일 내용을 읽고 buffer를 통해 전달
// 그러나 실제로는 파일은APP이 읽고 쓸 수 있는 스트림 인터페이스를 가진 모든 것을 말함. 즉, 파이프는 가상 파일이라고 할 수 있음
int pipes[2];

// 부모 프로세스는 작성자 역할함(읽기용 파이프 필요X)
void parent_proc(){
    char *buf = 0x0;
    ssize_t s; // 보내야 할 전체 데이터의 양
    size_t len = 0; // 이미 파이프로 보내진 데이터의 양

    close(pipes[0]); // 읽기용 파이프 필요없으므로 닫음
    
    // getline을 실행하여 표준 입력에서 한 문장을 가져와 buffer에 한 줄이 저장됨
    while((s = getline(&buf, &len, stdin)) != -1) {
        // buffer의 마지막 문자를 null로 바꾸어 문자열로 만들어줌
        buf[s -1] = 0x0;
        
        ssize_t sent = 0;
        char *date = buf;
        
        // OS는 제한된 양의 buffer를 가지고 있기에, app에서 보낸 데이터를 모두 가져올 수 없음
        // 무언가가 남아있다면 쓰기를 반복하여 모든 데이터를 보낼 때까지 계속 반복함
        while(sent < s) {
            // s - sent : 버퍼에 보내고자 함
            sent += write(pipes[1], buf + sent, s - sent);
        }

        free(buf);
        buf = 0x0;
        len = 0;
    }
    close(pipes[1]);
}

// 부모 프로세스는 데이터를 파이프로 전송
void child_proc() {
    char buf[32];
    ssize_t s;

    close(pipes[1]); // 자식 프로세스는 읽기만 하기에, 쓰기 파이프 닫음
    
    // 읽기 작업이 0을 반환한다는 것은 쓰기 작업이 완료되었음을 의미
    // 읽기 작업은 최대 31개의 문자를 한 번에 읽어 버퍼에 저장하고 출력함
    while((s = read(pipes[0], buf, 31)) > 0){
        buf[s + 1] = 0x0;
        printf(">%s\n", buf);
    }
    exit(0);
}

int main(void){
    pid_t child_pid;
    int exit_code;
    // pipe()는 파이프의 파일 디스크립터를 반환하여, 오류가 발생할 경우, -1 반환
    // 즉, 일반적으로 0이 아닌 반환값은 오류가 발생함을 의미
    // 또한, 생성된 파이프의 파일 디스크립터를 전역 변수로 저장
    // 내용이 배열에 저장
    if(pipe(pipes) != 0){
        perror("Error");
        exit(1);
    }
    printf("%d %d\n", pipes[0], pipes[1]);
    
    // 부모 프로세스에서 fork()가 호출되면 자식 프로세스 생성하여 부모 프로세스는 자식 프로세스의 ID를 받음
    // 부모와 자식 프로세스 모두 동일한 코드와 데이터를 가짐(메모리의 모든 것이 복제됨을 의미)
    // 자식 프로세스가 종료되고부모 프로세스도 종료되는 시점에서 부모 프로세스가 메모리를 복사하여 자식 프로세스가 부모 프로세스의 모든 것을 그대로 이어받음(부모 프로세스가 열었던 파일에 대한 정보에 대해  자식 프로세스가 그 파일에 대한 접근 권한 있음
    child_pid = fork();
   
   // child_pid의 값이 0일 경우, 자식 프로세스를 의미
    if(child_pid == 0) {
        child_proc();
    }
    // child_pid의 값이0이 아닐 경우, 부모 프로세스를 의미
    else{
        parent_proc();
    }
    wait(&exit_code);

    exit(0);
}
