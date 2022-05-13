# Register Allocation

by Charles Li

Local Register Allocation Project written in C

### Details

Register allocation has been implemented with 4 different flavors.
- Bottom-Up
- Top-Down Simple
- Top-Down with Max Live & Live Range Consideration
- Top-Down with Relative Frequency Ranking

### Usage 
In order to run the register allocator program:
1) Have the project directory open in terminal/shell

2) Place all testing input files in the directory
    (Alternatively, when running the program, you can provide the 
    absolute path to the input file if it exists in another directory)

3) Type `make` in the terminal and hit enter to compile the c program
   with gcc

4) To run the program, type `./alloc k f input.i` , where
    - k is the number of registers to test with
    - f is the allocation type flag: (b, s, t, o)
    - input.i is the input file or the path to the input file

5) With all the arguements typed out, hit enter to run
    - the program will output ILOC code with appropriate register allocation
