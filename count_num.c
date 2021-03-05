#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/wait.h>

#include <sys/resource.h>

#include <sys/types.h>

#include <fcntl.h>

#include <errno.h>

#include <time.h>



int main(int argc, char *argv[], char* env[]){



// Opening File and reading how many lines and how many lines per child

    FILE *fp;

    char ch;

    int linesCount = 0;



    fp = fopen(argv[1], "r");



    if (fp == NULL) {

        printf("Could not open file %s\n", argv[1]);

        exit(1);

    }



    //adds 1 to lineCount each time a \n is found untill End Of File

    while ((ch = fgetc(fp)) != EOF) {

        if (ch == '\n') {

            linesCount++;

        }

    }

    fclose(fp);

    int numOfChilds = atoi(argv[2]);

    //ceiling divsion

    int linesPerChild = (linesCount + numOfChilds -1) / numOfChilds;

//---------------------------------------------------------



//Making pipes

    int pipes[numOfChilds +1][2];

    int i;

    for (i = 0; i < numOfChilds + 1; i++) {

        if (pipe(pipes[i]) == -1) {

            printf("Error with created pipe\n");

        }

    }



//----------------------------------------



//Making N Children processes



    int rc[numOfChilds];



    for (int childi = 0; childi < numOfChilds; childi++) {

        rc[childi] = fork();

        if (rc[childi] < 0) {

            printf("fork failed\n");

            exit(1);

        }

        if (rc[childi] == 0) {

            //Child processes

            for (int pipesj = 0; pipesj <  numOfChilds +1; pipesj++) {

            //Close unused pipes------------------

                if (childi != pipesj) {

                    close(pipes[pipesj][0]);

                }

                if (childi+1 != pipesj) {

                    close(pipes[pipesj][1]);

                }

            }

            //-------------------------

            int startingLine;



            //number given in this pipe is line number to start on.

            if (read(pipes[childi][0], &startingLine, sizeof(int)) == -1) {

                printf("Error at reading\n");

                exit(1);

            }

            //printf("(%d) Got %d\n", childi, startingLine);



            // if the end of file is sooner then the line count make the correct value

            if ((linesPerChild + startingLine) > linesCount) {

                linesPerChild = ((startingLine - linesCount) * -1);

                 //printf("new number of lines %d\n", linesPerChild);

            }



            //Opening File for the Child to read

            fp = fopen(argv[1], "r");



            if (fp == NULL) {

                printf("Could not open file %s\n", argv[1]);

                exit(1);

            }



            //goes through the number of lines needed to skip throws with away with holder

            int holder;

            if (startingLine != 0) {

                for (int skippedLines = 0; skippedLines < startingLine; skippedLines++) {

                    fscanf (fp, "%d", &holder);

                }

            }

            //printf("%d \n", startingLine);





            // Variables used to save the numbers to an array

            int numbersArr[linesPerChild];

            int outputArr[linesPerChild][2];

            int linesRead = 0;

            int line;

            while (linesRead != linesPerChild) {

                fscanf (fp, "%d", &line);

                //printf("%d\n", line);

                numbersArr[linesRead] = line;

                linesRead++;

            }

            fclose(fp);

//---------------------------------



            int currentAppend = 0;

            for (int tempIndex = 0; tempIndex < linesPerChild; tempIndex++) {

                for (int twoDi = 0; twoDi < linesPerChild; twoDi++) {

                    //printf("im numArr %d , im outArr %d\n", numbersArr[tempIndex], outputArr[twoDi][0]);



                    if (twoDi == currentAppend) {

                        outputArr[currentAppend][0] = numbersArr[tempIndex];

                        outputArr[currentAppend][1] = 1;

                        currentAppend++;

                        break;

                    }



                    if (numbersArr[tempIndex] == outputArr[twoDi][0]) {

                        outputArr[twoDi][1] = (outputArr[twoDi][1]) +1;

                        break;

                    }

                }

            }



            int fdOut;

            if (childi == 0) {

                fdOut = open("childOutput", O_WRONLY|O_CREAT|O_TRUNC, 0664);

            } else {

                fdOut = open("childOutput", O_WRONLY|O_CREAT|O_APPEND, 0664);

            }





            dup2(fdOut, STDOUT_FILENO);





            for (int tempi = 0; tempi < currentAppend; tempi++) {

                    printf("%d ", outputArr[tempi][0]);

                    printf("%d\n", outputArr[tempi][1]);

            }



            close(fdOut);



            startingLine = startingLine + linesPerChild;

            //sends line number to start on for next process

            if (write(pipes[childi + 1][1], &startingLine, sizeof(int)) == -1) {

                exit(1);

            }

            //printf("(%d) Sent %d\n", i, x);

            close(pipes[childi][0]);

            close(pipes[childi+1][1]);



            // printf("hello, I am child (pid:%d)\n", (int) getpid());





            return 0;

        }

    }

//---------------------------------------------------------



//parent process

    int startingLine = 0;

    if (write(pipes[0][1], &startingLine, sizeof(int)) == -1) {

        printf("Error at writing\n");

        exit(1);

    }



    for (int i = 0; i < numOfChilds; i++) {

        while (wait(NULL) != -1 || errno != ECHILD);

    }



    int parentOut[linesCount][2];



    fp = fopen("childOutput", "r");



    if (fp == NULL) {

        printf("Could not open file %s\n", argv[1]);

        exit(1);

    }



    int lineCount = 0;

    while ((ch = fgetc(fp)) != EOF) {

        if (ch == '\n') {

            //printf("%d\n",ch);

            lineCount++;

        }

    }

    fclose(fp);



    fp = fopen("childOutput", "r");



    if (fp == NULL) {

        printf("Could not open file %s\n", argv[1]);

        exit(1);

    }



    int indexCount = 0;

    int line;

    while (indexCount < lineCount) {

        fscanf (fp, "%d", &line);

        //printf("%d i was taken 1st\n", line);

        parentOut[indexCount][0] = line;

        fscanf (fp, "%d", &line);

        //printf("%d i was taken 2nd\n", line);

        parentOut[indexCount][1] = line;

        indexCount++;

        //printf("index: %d , lines : %d\n", indexCount, lineCount);

    }



    fclose(fp);

    int fst;

    for (fst = 0; fst < indexCount; fst++) {

        for (int snd = 0; snd < indexCount; snd++) {

            if (parentOut[fst][0] == parentOut[snd][0] && fst != snd) {

                parentOut[fst][1] = parentOut[fst][1] + parentOut[snd][1];

                parentOut[snd][0] = -1;

            }

        }

    }

    for (int tempi = 0; tempi < indexCount; tempi++) {

        if (parentOut[tempi][0] != -1) {

            printf("%d ", parentOut[tempi][0]);

            printf("%d\n", parentOut[tempi][1]);

        }

    }



    return 0;

}