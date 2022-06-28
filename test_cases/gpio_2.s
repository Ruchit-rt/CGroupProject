mov r2,#1
lsl r2,#9
ldr r0,=0x20200000
str r2,[r0]
mov r1,#1
lsl r1,#3
str r1,[r0,#40]
mov r2,#10
loop:
str r1,[r0,#40]
sub r2,r2,#1
cmp r2,#0
str r1,[r0,#28]
bne loop
andeq r0,r0,r0
