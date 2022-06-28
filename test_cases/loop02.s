mov r2,#5
wait:
sub r2,r2,#1
mov r1,#10
wait1:
sub r1,r1,#1
cmp r1,r2
bne wait1
cmp r2,#0
bne wait
