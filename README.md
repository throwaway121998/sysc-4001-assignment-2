# sysc-4001-assignment-2

This repository contains our solution to assignment 2.

## Usage

Run the Makefile.

## Tests
Sample output from the mandatory test cases:
```
The shared mem is attached @ location: 0x4D7000
Enter a # for arr[0]: 5
Enter a # for arr[1]: 6
Enter a # for arr[2]: 8
Enter a # for arr[3]: 2
Enter a # for arr[4]: 7
Result: [8, 7, 6, 5, 2]
Min value: 2
Median value: 6
Max value: 8
Program ended with exit code: 0
```
```
The shared mem is attached @ location: 0x4D7000
Enter a # for arr[0]: 10
Enter a # for arr[1]: 9
Enter a # for arr[2]: 11
Enter a # for arr[3]: 5
Enter a # for arr[4]: 7
Result: [11, 10, 9, 7, 5]
Min value: 5
Median value: 9
Max value: 11
Program ended with exit code: 0
```

## Discussion

Our solution uses SZ processes and semaphores, where SZ is the number of elements in the array, B. Processes [0, ..., SZ - 2] are tasked with manipulating the array B at index [i, i + 1], where i is the process number. To avoid deadlocks, our solution uses asymmetry: even number processes await semaphores i then i + 1 whilst odd processes await semaphores i + 1 then i. Finally, process number SZ - 1, is tasked with checking to see if the array is successfully placed in decreasing order. If the array is placed in decreasing order, then process SZ - 1 set's the shared state `is_alive` to false, and all processes break and exit. The pseudocode is as follows:

### Pseudocode

```
Instantiate the shared memory.
Make the shared memory accessible to the program.
Instantiate the array, B, of size SZ from user input.
Instantiate SZ semaphores.
Set is_alive to true.
Fork SZ processes.
If a parent process then break.
If a child process then: 
  Loop while is_alive:
    If process number is SZ - 1 then:
      If the array is in decreasing order then set is_alive to false.
    If process number is [0, ..., SZ - 2] then:
      If i is even then:
        Await semaphore i.
        Await semaphore i + 1.
        If B[i] < B[i + 1] then swap B[i] and B[i + 1].
        Signal semaphore i.
        Signal semaphore i + 1.
      If i is odd then:
        Await semaphore i + 1.
        Await semaphore i.
        If B[i] < B[i + 1] then swap B[i] and B[i + 1].
        Signal semaphore i + 1.
        Signal semaphore i.
  Exit process.
Await SZ processes to exit.
Print the reconstructed array.
Print the min value.
Print the median value.
Print the max value.
Detach and delete the shared memory.
         
```



## Contributors

* Kyle Horne 101038042
* Dmytro Sytnik 101020533
* Joel Roy-Couvillon 100967575
