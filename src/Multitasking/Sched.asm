BITS 64

section .text

%include "Include/Common.inc"

; void TaskSwitch(struct Registers *regs, uint8_t *ext_regs);

global TaskSwitch

TaskSwitch:
	MOV RSP, RDI

	POPAQ

	ADD RSP, 0x10

	IRETQ

extern Schedule

; void _Yield();

global _Yield

_Yield:
	MOV RSI, QWORD [RSP]
	ADD RSP, 0x08
	MOV RAX, RSP

	MOV RDI, SS

	PUSH RDI
	PUSH RAX
	PUSHFQ

	MOV RAX, CS
	PUSH RAX

	PUSH RSI

	SUB RSP, 0x10
	PUSHAQ

	MOV RDI, RSP
	CLD
	CALL Schedule

	UD2
