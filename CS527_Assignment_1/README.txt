CS527 Assignment 1 - Mini Computer Simulator

Files included:
C source code: main.c, compiler.c, processor.c, memory.c
Header files: compiler.h, processor.h, memory.h
Test cases: faq1.txt, faq2.txt, faq3.txt

How to compile and run:
1. Open the terminal in this folder.
2. Compile the code using gcc:
gcc main.c compiler.c processor.c memory.c -o simulator.exe

3. Run the executable:
.\simulator.exe

4. The program will show a menu. Enter 1, 2, or 3 to select a test case:
Option 1 runs faq1.txt (Sum of N numbers)
Option 2 runs faq2.txt (Multiply two complex numbers)
Option 3 runs faq3.txt (Determinant of a 3x3 matrix)

Notes:
The compiler will read the selected text file and generate program.byte.
The processor will execute the byte file and print the opcode details and final register values to the screen.