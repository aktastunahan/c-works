**This repository contains my work on learning some advanced topics using the C language.**


# HW-Bank-Branch: Simulation of a Bank Branch with Multiple Pay Desks and Clients
| Arguments | Explanation | Default |
| --------- | :-----| :-----|
| -c |  The total number of clients to be generated   | 20  |
| -n |  Number of pay desk threads                    | 4   |
| -q |  The maximum size of the queues                | 3   |
| -g |  The rate of generation time                   | 100 |
| -d |  The rate of duration time                     | 10  |

+  Compile the code by `gcc main.c -o office -lm -pthread`
  *  Example run: `./office –c 7 –n 2 –q 2 –g 25 –d 5` <br /> <br />
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
