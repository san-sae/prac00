#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<sys/time.h>

int main(int argc, char *argv[]){
    int opt;
    char *inputdir, *outputdir, *targetsrc;
    int timelimit;
    
    // command-line 유효성 검사
    while((opt = getopt(argc, argv, "i:a:t")) != -1){
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


    int total_inputs = 0;
    int success_count = 0;
    int timeout_count = 0;
    int crash_count = 0;
    int wrong_answer_count = 0;

    // 타임 스탬프
    struct timeval start_time, end_time;
    double diff_time;

    gettimeofday(&start_time, NULL);

    for(int i = 0; i<=20; i++){
        char input_file[100];
        sprintf(input_file, "%s/%d.txt", inputdir, i);
        
        // Create pipe for communication
        int fd[2];
        if(pipe(fd) == -1)/ Fork child process
        pid_t pid = fork();
        if(pid == -1){
            perror("Fork Error");
            exit(EXIT_FAILURE);
        }
}
