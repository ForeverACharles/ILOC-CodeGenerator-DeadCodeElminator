# ILOC Register Allocation with Dead Opcode Eliminator

### Overview

<p>
    <img src="https://user-images.githubusercontent.com/50348516/168185683-10860c41-ec1d-4dad-bf65-5ac4db4c4907.png" align="right" "width="300"/> <img src="https://user-images.githubusercontent.com/50348516/168185749-ecebaeff-91d7-49e1-bee0-6a346d77fa7e.png" align="right" height="300" width="250"/> </p>
    
The ILOC instruction set has similar feature parity with the more widely-known Assembly instruction set.
In C code, the compiler takes the higher level language and translates it into a lower level language of Assembly. Which then gets translated to machine code of just 1s and 0s.

One of the key roles the compiler is to decide how to allocate physical CPU registers most efficiently as it does its translation. If the higher level language involves many arithemetic and memory operations, then it is advatangeous to have as many physical registers as  possible to account for all of them. But in the real world, processors only have a limited amount.

So to solve this issue, the compiler must dedicate a certain number of the physical registers to act as virtual registers. The job of these registers is to store values temporarily so that the normal physical registers can perform any important arithmetic operations before fetching the values that are stored in the virtual ones.

This repo is broken up into 2 parts:
- ILOC physical & virtual register allocation
- ILOC dead opcode eliminator
