//2 + 2 is 4, minus 1 that's 3 quick maffs
loadI 1024 => r0
loadI 2 => r1
loadI 2 => r2
//storeAI r1 => r0, -4
add r1, r2 => r3
//storeAI r2 => r0, -8
loadI 1 => r4
sub r3, r4 => r5
store r5 => r0
add r1, r1 => r6
//r6 = 2 + 2 = 4
addI r1, 5 => r8
//r7 = 2 + 5 = 7
output 1024
store r6 => r0
output 1024
store r8 => r0
output 1024
add r1, r2 => r3
store r3 => r0
output 1024
