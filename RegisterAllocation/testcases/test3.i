loadI	1024	=> r0
loadI 1036 => r12

loadI 2 => r1
loadI 2 => r2
add r1, r2 => r3
loadI 1028 => r4
store r3 => r4

add r1, r1 => r10
store r10 => r12
add r2, r2 => r13
load r12 => r14
add r14, r13 => r15
store r15 => r12

load r4 => r5
loadI 1 => r6
sub r5, r6 => r7
loadI 1032 => r8
store r7 => r8

output 1028
output 1036
output 1032