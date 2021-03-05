# child Processes Counter
Author: Blake Berringer

## run through command line 

Suppose your program is compile to ‘a.out’. If you run ‘./a.out input_filename 5’, the program should create 5 child processes to do the task. Use ‘g++ -std=c++11 count_num.c’ to compile the code.


counts the number of times a number occurs in an input file -- mostly used for practice with Child and parent processes.

 C program that creates N child processes and use the child processes to count number in a file. The input file has one number in each line, and different lines can have identical numbers. Given N, you need to divide the input file evenly into N parts and count the numbers in each part with a child process. For example, an input file may look like:
 
1

2

2

10

5

3

10

If N=2, the file will be divided into two parts: the first four lines and the last three lines. The output of the first child process is

1 1

2 2

10 1

 

It means that 1 appears once, 2 appears twice, and 10 appears once in the first four lines.

Similarly, the output of the second process is

5 1

3 1

10 1

 

Each of the child processes will write the output pairs into an output file. After all child process finish, the main process will read in the output files created by the child processes and merge the results. In the above example, the main process will read in the outputs for the two parts and print the final result to console.

1 1

2 2

3 1

5 1

10 2

The order of the numbers in the final result does not matter.
