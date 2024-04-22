#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<string.h>
#include<limits.h>
#include<signal.h>

char *inputdir, *answerdir, *targetsrc;
int timelimit;

void validation(int argc, char *argv[]){
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
                fprintf(stderr, "Usage: %s -i <inputdir> -a <answer> -t <timelimit> <target src>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if(argc - optind != 1){
        fprintf(stderr, "Usage: %s -i <inputdir> -a <answer> -t <timelimit> <target src>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

}

int main(int argc, char *argv[] ){
    validation(argc, argv);
    
    targetsrc = argv[optind];
    
    struct timeval start, end;
    long mtime;

    gettimeofday(&start, NULL);
    


    usleep(1000000);




    gettimeofday(&end, NULL);
    mtime = (end.tv_sec - start.tv_sec)*1000 + ((end.tv_usec - start.tv_usec)/1000.0);

    if(mtime > timelimit){
        printf("Timeout occured");
    }
    else{
        printf("%ld\n", mtime);
    }


    return 0;
}
