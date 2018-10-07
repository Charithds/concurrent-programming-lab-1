# concurrent-programming-lab-1
This repo is dedicated for "Lab 1" of CS4532 Concurrent Programming. In this lab we will learn how to develop parallel programs using the Pthread library. We will also compare the performance of different implementations.

## Compiling
gcc Lab_1.c LinkedList.c LinkedList.h -lm -lpthread -o Lab_1.out

## Running the program
Usage: Lab_1.out <mMember> <mInsert> <mDelete> <thread_count> <prog_type (M,R or S)>
For the serial program <thread_count> will be ignored

Ex: ./Lab_1.out 9000 500 500 1 S

