#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/time.h>
#include<limits.h>

#define MAX_FILES 20

char *inputdir, *answerdir, *targetsrc;
int timelimit;

void  validation_opt(int argc, char *argv[]){
    int option;

    while((option = getopt(argc, argv, "i:a:t:")) != -1){
        switch(option){
            case 'i':
                inputdir = optarg;
                break;
            case 'a':
                answerdir = optarg;
                break;
            case 't':
                timelimit = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: ./%s -i <inputdir> -a <answerdir> -t <timelimit> <target src>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if(argc - optind != 1){
        fprintf(stderr, "Usage: ./%s -i <inputdir> -a <answerdir> -t <timelimit> <target src>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}
            
int countExistingFiles(const char *directory){
    int pipes[2];
    pid_t pid;
    int existingFiles = 0;

    if(pipe(pipes) == -1){
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if(pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0){ // 자식프로세스
        close(pipes[0]);
        
        for(int i = 1; i <= MAX_FILES; i++){
            char filename[20];
            sprintf(filename, "%s/%d.txt", directory, i);
            // dir_name/i.txt가 존재하는 경우
            if(access(filename, F_OK) != -1){
                existingFiles++;
            }
        }

        // 부모 프로세스로 파일 존재 여부 정보 전송
        write(pipes[1], &existingFiles, sizeof(existingFiles));

        close(pipes[1]);

        exit(EXIT_SUCCESS);
    }
    else{ // 부모 프로세스
        int exit_code;
        close(pipes[1]);
        
        int count = 0;

        read(pipes[0], &count, sizeof(count));

        close(pipes[0]);

        wait(&exit_code);

        return count;
    }
}

void compileFile(const char *directory, const char *filename){
    char dir_filename[100];
    sprintf(dir_filename, "%s/%s", directory, filename); 
    char *args[] = {"gcc", "-fsanitize=address", dir_filename, "-o", filename, NULL};

    if(execvp("gcc", args) == -1){
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
}

void compileAndExec(const char *directory, int num_files){
    // 컴파일
    for(int i = 1; i <= num_files; i++){
        char filename[10];
        sprintf(filename, "%d.txt", i);

        pid_t pid = fork();

        if(pid < 0){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            compileFile(directory, filename);
            exit(EXIT_SUCCESS);
        }
        else{
            int status;
            wait(&status);
        }
    }
}

int main(int argc, char *argv[]){

    // 명령행  유효성 검사 함수 호출
    validation_opt(argc, argv);

    // input, answer 디렉토리 내에 파일이 존재하는지 여부 체크
    int numInputFiles = countExistingFiles(inputdir);
    int numAnswerFiles = countExistingFiles(answerdir);

    compileAndExec(inputdir, numInputFiles);
    
    printf("Compile completed\n");







/*
    targetsrc = argv[optind];

    int total_inputs = 0;
    int success_count = 0;
    int timeout_count = 0;
    int crash_count = 0;
    int wrong_answer_count = 0;

    // 타임 스탬프
    struct timeval start_time, end_time;
    double diff_time;

    gettimeofday(&start_time, NULL);

    for(int i = 1; i<=10; i++){
        char input_file[100];
        sprintf(input_file, "%s/%d.txt", inputdir, i);
        
        // Create pipe for communication
        int fd[2];
        if(pipe(fd) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        // Fork child process
        pid_t pid = fork();
        if(pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if(pid == 0){ // child process
            //close read end of pipe
            close(fd[0]);

            // redirect stdout to the write end of the pipe
            if(dup2(fd[1], STDOUT_FILENO) == -1){
                perror("dup2");
            }
            close(fd[1]);
*/

            // Execute the target program
            /* execlp()의 argument
             * 1st. 실행할 프로그램의 경로 지정
             * 2nd. 실행할 프로그램명
             * 3rd. 실행할 프로그램이 필요로 하는 입력 또는 설정 전달
             * 4th. 추가적인 실행 인자, 마지막 인자는 항상 NULL로 설정하며 이는 인자 목록의 끝을 의미함
            */
/*            execlp(targetsrc, targetsrc, input_file, NULL);
            perror("excelp");
            exit(EXIT_FAILURE);
        }
        else{ // parent process
            // close write end of pipe
            close(fd[1]);

            // read output from the child process
            char buffer[4096]; // Adjust buffer size as needed
            ssize_t bytes_read = read(fd[0], buffer, sizeof(buffer));
            if(bytes_read == -1){
                perror("read");
                exit(EXIT_FAILURE);
            }
            close(fd[0]);

            // check if execution exceeded time limit
            gettimeofday(&end_time, NULL);
            // 밀리초 단위로실행시간 출력
            diff_time = (end_time.tv_sec - start_time.tv_sec) *1000 + ((end_time.tv_usec - start_time.tv_usec) / 1000);
            if(diff_time > timelimit){
                printf("Timeout occurred for input %d\n", i);
                timeout_count++;
                kill(pid, SIGKILL);
            }
            else{
                // Wait for child process to finish
                int status;
                waitpid(pid, &status, 0);
                if(WIFEXITED(status)){
                    int exit_status = WEXITSTATUS(status);
                    if(exit_status == 0){
                        // Program executed successfully
                        char output_str[4096]; // Adjust buffer size as needed
                        ssize_t bytes_read_pipe = read(fd[0], output_str, sizeof(output_str));
                        if(bytes_read_pipe == -1){
                            perror("read");
                            exit(EXIT_FAILURE);
                        }

                        // Compare output with expected output
                        if(bytes_read == bytes_read_pipe && memcmp(buffer, output_str, bytes_read) == 0){
                            printf("Success for input %d\n", i);
                            success_count++;
                        }
                        else{
                            printf("Wrong answer for input %d\n", i);
                            wrong_answer_count++;
                        }
                    }
                    else{
                        // runtime Error
                        printf("Runtime error occurred for input %d\n", i);
                        crash_count++;
                    }
                }
            }
        }
        total_inputs++;
    }
    printf("Total inputs: %d\n", total_inputs);
    printf("Success count: %d\n", success_count);
    printf("Timeout count: %d\n", timeout_count);
    printf("Crash count: %d\n", crash_count);
    printf("Wrong answer count: %d\n", wrong_answer_count);
*/
    return 0;
}
