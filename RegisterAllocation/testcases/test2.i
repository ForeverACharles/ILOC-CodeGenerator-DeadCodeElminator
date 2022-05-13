// CS 415 Project #1 - block1.i
// 
// Just a long random computation
// Expects two inputs at locations 1024 and 1028 -
// the first is the initial value used in the computation and 
// the second is the incrementor.
//
// Example usage: sim -i 1024 1 1 < block1.i

	loadI	1024	=> r0
	loadI	1032	=> r1
	loadI	1024	=> r10
	load	r10	=> r11
	loadI	4	=> r12
	loadI	1028	=> r13
	load	r13	=> r14

	store	r11	=> r1
	add	r1, r12	=> r15
	add	r11, r14	=> r16
	store	r16	=> r15

	add	r15, r12	=> r17
	add	r16, r14	=> r18
	store	r18	=> r17

    output 1032