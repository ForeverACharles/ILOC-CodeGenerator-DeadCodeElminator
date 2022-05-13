# ILOC Register Allocation with Dead Opcode Eliminator

### ILOC Overview

<p>
    <img src="https://user-images.githubusercontent.com/50348516/168185749-ecebaeff-91d7-49e1-bee0-6a346d77fa7e.png" align="right" height="325" width="250"/> <img src="https://user-images.githubusercontent.com/50348516/168185683-10860c41-ec1d-4dad-bf65-5ac4db4c4907.png" align="right" height="225" width="225"/>  </p>
    
The ILOC instruction set has similar feature parity with the more widely-known Assembly instruction set.
In C code, the compiler takes the higher level language and translates it into a lower level language of Assembly. Which then gets translated to machine code of just 1s and 0s.

### Register Allocation

One of the key roles of the compiler is decidng how to allocate physical CPU registers most efficiently as it does its translation. If the higher level language involves many arithemetic and memory operations, then it is advatangeous to have as many physical registers as possible to account for all of them. But in the real world, processors only have a limited amount.

So to solve this issue, the compiler must dedicate a certain number of available physical registers to act as virtual registers. The job of these virtual registers is to give somewhere for the CPU to offload values that aren't immediately needed so that the normal physical registers can be filled with values that are immediately needed for arithmetic operations.

Whenever the compiler needs to spill values from physcial registers to virtual registers, it must produce respective spill code. Spill code inherently has a performance penalty as it implies that processing time for artihmetic operations are instead spent on register-to-register transfers. Minimizing the amount of spill code produced during compilation is crucial for efficiency.

### Dead Code Elimination

This repo is broken up into 2 parts:
- ILOC physical & virtual register allocation
- ILOC dead opcode eliminator
