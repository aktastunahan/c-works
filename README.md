**This repository contains my work on learning some advanced topics using the C language.**

# User Level Thread Scheduling
A simulation of thread scheduling in user level, using `ucontext.h` library. Shortest time remaining time (srtf) and lottery ticket schedular functions are avaliable.<br />
Each thread is required to execute a simple counter function that takes two arguments, “n” and “i”, n being the stopping criteria for counting and i being the thread number. The function counts up starting from zero up to “n”. With each increment, the function prints the count value having “i” tabs on its left. After each print, the function sleeps for 1 seconds. The program takes "n" of each thread as input.
+  Compile the code by `gcc main.c -o sch`
   *  Lottery schedular: `./sch p_wf <share_1> <share_2> ... <share_n>`
      -  Example run: `./sch p_wf 1 3 5` <br /> <br />`
         ![](https://github.com/aktastunahan/c-works/blob/main/User-Level-Thread-Scheduling/lottery_ticket_sch_example.PNG)
   *  SRTF schedular: `./sch srtf <share_1> <share_2> ... <share_n>`
      -  Example run: `./sch srtf 1 3 5` <br /> <br />`
         ![](https://github.com/aktastunahan/c-works/blob/main/User-Level-Thread-Scheduling/srtf_sch_example.PNG)

# HW-Bank-Branch: Simulation of a Bank Branch with Multiple Pay Desks and Clients
| Arguments | Explanation | Default |
| --------- | :-----| :-----|
| -c |  The total number of clients to be generated   | 20  |
| -n |  Number of pay desk threads                    | 4   |
| -q |  The maximum size of the queues                | 3   |
| -g |  The rate of generation time                   | 100 |
| -d |  The rate of duration time                     | 10  |

+  Compile the code by `gcc main.c -o office -lm -pthread`
  *  Example run: `./office` <br /> <br />
     ![](https://github.com/aktastunahan/c-works/blob/main/HW-Bank-Branch/bank_branch_1.PNG)
  *  Example run: `./office –c 7 –n 2 –q 2 –g 25 –d 5` <br /> <br />
     ![](https://github.com/aktastunahan/c-works/blob/main/HW-Bank-Branch/bank_branch_2.PNG)

# hw1: Count Words by Multithreading
Two different programs to count words in a given file. One counts the words
1.  Compile the source code using `make` command
2.  Run: `./pwords <file_path_1> <file_path_2> ... <file_path_n>`. For each file, a seperate thread counts the words.
## Example
![](https://github.com/aktastunahan/c-works/blob/main/hw1/img1.PNG)
![](https://github.com/aktastunahan/c-works/blob/main/hw1/img2.PNG)
![](https://github.com/aktastunahan/c-works/blob/main/hw1/img3.PNG)
