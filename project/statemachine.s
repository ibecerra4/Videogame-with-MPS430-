	.arch msp430g2553
	.p2align 1,0
	.text
	
JmpTable:
	.word case0
	.word case1
	.word case2
	.word case3
	.word case4

	.global playNote
playNote:
	mov &sound, r12
	add r12, r12
	mov JmpTable(r12), r0
case0:
	mov #0, r12
	jmp end
case1:
	mov #1200, r12
	jmp end
case2:
	mov #1300, r12
	jmp end
case3:
	mov #1400, r12
	jmp end
case4:
	mov #1500, r12
	jmp end
end:
	ret
