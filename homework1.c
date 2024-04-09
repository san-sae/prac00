// $ dirname /home/user/test.c   => /home/user
// $ basename /home/user/test.c  => test.c

// $ ./a.out < input/1.txt
// $ ./a.out < input/1.txt > out
// $ diff out answer/1.txt
// ...
// $ ./a.out < input/7.txt > out => 무한루프에 빠짐(시간초과의 사례)
// $ ./a.out < input/8.txt > out => Segmentation fault (core dumped)
    // segmentation fault 발생 시, 실제로 크래시가 발생했는지 확인하는 방법
    // 1. 표준 오류 출력 확인(쉘에서 명령어 입력 후 나타나는 출력 ex. segmentation fault)
    // 2. $ echo $? (최신 프로그램 실행의 종료코드 반환하는 명령어)
    //                  0이 아닌 종료코드는 문제가 생겼다는 것
// $ ./a.out < input/9.txt => stack smashing detected(버퍼 오버런)
    // 컴파일러 돌려보기 : $ gcc -fsanitize=address pathname.c(숨겨진 버퍼플로우를 발견가능)
// $ ./a.out < input/10.txt => 이중 슬래시로 오류가 있으나, 오류가 발견되지 않음

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <dirent.h>
#include <getopt.h>

#define MAX_INPUT_LEN 1000
#define MAX_FILENAME_LEN 256
#define MAX_TEST_CASES 20

typedef struct {
    char input[MAX_FILENAME_LEN];
    char answer[MAX_FILENAME_LEN];
} TestCase;

int compile_program(char *src_file) {
    pid_t pid;
    int status;
    
    if ((pid = fork()) == 0) {
        execlp("gcc", "gcc", "-fsanitize=address", src_file, "-o", "target_program", NULL);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                return 1;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }
}

int execute_program(char *input_file, int time_limit) {
    pid_t pid;
    int fd[2];
    struct timeval start, end;
    
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
    
    if ((pid = fork()) == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        
        gettimeofday(&start, NULL);
        execl("./target_program", "./target_program", NULL);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else {
        close(fd[0]);
        FILE *file = fopen(input_file, "r");
        if (!file) {
            perror("Failed to open input file");
            exit(EXIT_FAILURE);
        }
        
        char buffer[MAX_INPUT_LEN];
        while (fgets(buffer, MAX_INPUT_LEN, file) != NULL) {
            write(fd[1], buffer, strlen(buffer));
        }
        close(fd[1]);
        
        int status;
        waitpid(pid, &status, 0);
        gettimeofday(&end, NULL);
        
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                int elapsed_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
                return elapsed_time;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }
}

int compare_output(char *answer_file) {
    FILE *output_file = popen("./target_program", "r");
    if (!output_file) {
        perror("Failed to open program output");
        exit(EXIT_FAILURE);
    }
    
    FILE *answer = fopen(answer_file, "r");
    if (!answer) {
        perror("Failed to open answer file");
        exit(EXIT_FAILURE);
    }
    
    int match = 1;
    char output_line[MAX_INPUT_LEN], answer_line[MAX_INPUT_LEN];
    while (fgets(output_line, MAX_INPUT_LEN, output_file) != NULL &&
           fgets(answer_line, MAX_INPUT_LEN, answer) != NULL) {
        if (strcmp(output_line, answer_line) != 0) {
            match = 0;
            break;
        }
    }
    
    pclose(output_file);
    fclose(answer);
    
    return match;
}

int main(int argc, char *argv[]) {
    char *input_dir, *answer_dir, *src_file;
    int time_limit;
    
    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "i:a:t:")) != -1) {
        switch (opt) {
            case 'i':
                input_dir = optarg;
                break;
            case 'a':
                answer_dir = optarg;
                break;
            case 't':
                time_limit = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -i <inputdir> -a <outputdir> -t <timelimit> <target_src>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Usage: %s -i <inputdir> -a <outputdir> -t <timelimit> <target_src>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    src_file = argv[optind];
    
    // Read input and answer files
    TestCase test_cases[MAX_TEST_CASES];
    int num_test_cases = 0;
    DIR *dir;
    struct dirent *entry;
    
    if (!(dir = opendir(input_dir))) {
        perror("Failed to open input directory");
        exit(EXIT_FAILURE);
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            snprintf(test_cases[num_test_cases].input, MAX_FILENAME_LEN, "%s/%s", input_dir, entry->d_name);
            snprintf(test_cases[num_test_cases].answer, MAX_FILENAME_LEN, "%s/%s", answer_dir, entry->d_name);
            num_test_cases++;
        }
    }
    closedir(dir);
    
    // Compile the target program
    if (!compile_program(src_file)) {
        fprintf(stderr, "Compilation failed\n");
        exit(EXIT_FAILURE);
    }
    
    // Run test cases
    int correct = 0, timeout = 0, wrong = 0;
    int elapsed_time_sum = 0;
    for (int i = 0; i < num_test_cases; i++) {
        int elapsed_time = execute_program(test_cases[i].input, time_limit);
        if (elapsed_time == -1) {
            timeout++;
        } else {
            int match = compare_output(test_cases[i].answer);
            if (match) {
                correct++;
                elapsed_time_sum += elapsed_time;
            } else {
                wrong++;
            }
        }
    }
    
    // Print results
    if (wrong > 0) {
        printf("Wrong Answer (%d/%d): %d correct outputs, %d wrong outputs\n", correct, num_test_cases, correct, wrong);
    } else if (timeout > 0) {
        printf("Timeout: %d correct outputs, %d timeouts\n", correct, timeout);
    } else {
        printf("Correct: Sum of running time of all test executions: %d milliseconds\n", elapsed_time_sum);
    }
    
    return 0;
}

