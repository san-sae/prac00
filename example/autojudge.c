#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/time.h>
#include<limits.h>

int main(int argc, char *argv[]){
    int opt;
    char *inputdir, *outputdir, *targetsrc;
    int timelimit;
    
    // command-line 유효성 검사
    while((opt = getopt(argc, argv, "i:a:t:")) != -1){
        switch(opt){
            case 'i':
                inputdir = optarg;
                break;
            case 'a':
                outputdir = optarg;
                break;
            case 't':
                timelimit = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -i <inputdir> -a <outputdir> -t <timelimit> <target src>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if(argc - optind != 1) { 
        fprintf(stderr, "Usage: %s -i <inputdir> -a <outputdir> -t <timelimit> <target src>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    targetsrc = argv[optind];
    
    char absolute_path[PATH_MAX];
    if(realpath(targetsrc, absolute_path) == NULL){
        perror("realpath");
        exit(EXIT_FAILURE);
    }

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
        sprintf(input_file, "./%s/%d.txt", inputdir, i);
        
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

            // Execute the target program
            /* execlp()의 argument
             * 1st. 실행할 프로그램의 경로 지정
             * 2nd. 실행할 프로그램명
             * 3rd. 실행할 프로그램이 필요로 하는 입력 또는 설정 전달
             * 4th. 추가적인 실행 인자, 마지막 인자는 항상 NULL로 설정하며 이는 인자 목록의 끝을 의미함
            */
            execlp(targetsrc, targetsrc, input_file, NULL);
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

    return 0;
}
