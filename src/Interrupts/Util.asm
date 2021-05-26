BITS 64

%include "Include/Common.inc"

section .text

; void GDTFlush(void*)

global GDTFlush

GDTFlush:
	LGDT [RDI]

	PUSH RBP
	MOV RBP, RSP

	PUSH QWORD 0x10
	PUSH QWORD RBP
	PUSHFQ
	PUSH QWORD 0x08
	PUSH QWORD .L0

	IRETQ
.L0:
	POP RBP

	MOV AX, 0x10

	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	RET

extern idt_handlers

; void IDTCommonHandlerN();

%macro IDT_HANDLER_A 1 ; Handler A does push an error code to the stack

global IDTCommonHandler%1

IDTCommonHandler%1:
	PUSH QWORD 0
	PUSH QWORD %1
	PUSHAQ

	CLD

	MOV RDI, RSP ; Each handler takes a struct Registers* as an argument

	MOV RAX, QWORD [idt_handlers + %1 * 8] ; Get the handler into RAX
	CALL RAX

	POPAQ
	ADD RSP, 0x10 ; Pop the error code and the vector number off the stack

	IRETQ
%endmacro

%macro IDT_HANDLER_B 1 ; Handler B does not push an error code to the stack

global IDTCommonHandler%1

IDTCommonHandler%1:
	PUSH QWORD %1
	PUSHAQ

	CLD

	MOV RDI, RSP ; Each handler takes a struct Registers* as an argument

	MOV RAX, QWORD [idt_handlers + %1 * 8] ; Get the handler into RAX
	CALL RAX

	POPAQ
	ADD RSP, 0x10 ; Pop the error code and the vector number off the stack

	IRETQ
%endmacro

IDT_HANDLER_A 0
IDT_HANDLER_A 1
IDT_HANDLER_A 2
IDT_HANDLER_A 3
IDT_HANDLER_A 4
IDT_HANDLER_A 5
IDT_HANDLER_A 6
IDT_HANDLER_A 7
IDT_HANDLER_B 8
IDT_HANDLER_A 9
IDT_HANDLER_B 10
IDT_HANDLER_B 11
IDT_HANDLER_B 12
IDT_HANDLER_B 13
IDT_HANDLER_B 14
IDT_HANDLER_A 15
IDT_HANDLER_A 16
IDT_HANDLER_B 17
IDT_HANDLER_A 18
IDT_HANDLER_A 19
IDT_HANDLER_A 20
IDT_HANDLER_A 21
IDT_HANDLER_A 22
IDT_HANDLER_A 23
IDT_HANDLER_A 24
IDT_HANDLER_A 25
IDT_HANDLER_A 26
IDT_HANDLER_A 27
IDT_HANDLER_A 28
IDT_HANDLER_A 29
IDT_HANDLER_B 30
IDT_HANDLER_A 31
IDT_HANDLER_A 32
IDT_HANDLER_A 33
IDT_HANDLER_A 34
IDT_HANDLER_A 35
IDT_HANDLER_A 36
IDT_HANDLER_A 37
IDT_HANDLER_A 38
IDT_HANDLER_A 39
IDT_HANDLER_A 40
IDT_HANDLER_A 41
IDT_HANDLER_A 42
IDT_HANDLER_A 43
IDT_HANDLER_A 44
IDT_HANDLER_A 45
IDT_HANDLER_A 46
IDT_HANDLER_A 47
IDT_HANDLER_A 48
IDT_HANDLER_A 49
IDT_HANDLER_A 50
IDT_HANDLER_A 51
IDT_HANDLER_A 52
IDT_HANDLER_A 53
IDT_HANDLER_A 54
IDT_HANDLER_A 55
IDT_HANDLER_A 56
IDT_HANDLER_A 57
IDT_HANDLER_A 58
IDT_HANDLER_A 59
IDT_HANDLER_A 60
IDT_HANDLER_A 61
IDT_HANDLER_A 62
IDT_HANDLER_A 63
IDT_HANDLER_A 64
IDT_HANDLER_A 65
IDT_HANDLER_A 66
IDT_HANDLER_A 67
IDT_HANDLER_A 68
IDT_HANDLER_A 69
IDT_HANDLER_A 70
IDT_HANDLER_A 71
IDT_HANDLER_A 72
IDT_HANDLER_A 73
IDT_HANDLER_A 74
IDT_HANDLER_A 75
IDT_HANDLER_A 76
IDT_HANDLER_A 77
IDT_HANDLER_A 78
IDT_HANDLER_A 79
IDT_HANDLER_A 80
IDT_HANDLER_A 81
IDT_HANDLER_A 82
IDT_HANDLER_A 83
IDT_HANDLER_A 84
IDT_HANDLER_A 85
IDT_HANDLER_A 86
IDT_HANDLER_A 87
IDT_HANDLER_A 88
IDT_HANDLER_A 89
IDT_HANDLER_A 90
IDT_HANDLER_A 91
IDT_HANDLER_A 92
IDT_HANDLER_A 93
IDT_HANDLER_A 94
IDT_HANDLER_A 95
IDT_HANDLER_A 96
IDT_HANDLER_A 97
IDT_HANDLER_A 98
IDT_HANDLER_A 99
IDT_HANDLER_A 100
IDT_HANDLER_A 101
IDT_HANDLER_A 102
IDT_HANDLER_A 103
IDT_HANDLER_A 104
IDT_HANDLER_A 105
IDT_HANDLER_A 106
IDT_HANDLER_A 107
IDT_HANDLER_A 108
IDT_HANDLER_A 109
IDT_HANDLER_A 110
IDT_HANDLER_A 111
IDT_HANDLER_A 112
IDT_HANDLER_A 113
IDT_HANDLER_A 114
IDT_HANDLER_A 115
IDT_HANDLER_A 116
IDT_HANDLER_A 117
IDT_HANDLER_A 118
IDT_HANDLER_A 119
IDT_HANDLER_A 120
IDT_HANDLER_A 121
IDT_HANDLER_A 122
IDT_HANDLER_A 123
IDT_HANDLER_A 124
IDT_HANDLER_A 125
IDT_HANDLER_A 126
IDT_HANDLER_A 127
IDT_HANDLER_A 128
IDT_HANDLER_A 129
IDT_HANDLER_A 130
IDT_HANDLER_A 131
IDT_HANDLER_A 132
IDT_HANDLER_A 133
IDT_HANDLER_A 134
IDT_HANDLER_A 135
IDT_HANDLER_A 136
IDT_HANDLER_A 137
IDT_HANDLER_A 138
IDT_HANDLER_A 139
IDT_HANDLER_A 140
IDT_HANDLER_A 141
IDT_HANDLER_A 142
IDT_HANDLER_A 143
IDT_HANDLER_A 144
IDT_HANDLER_A 145
IDT_HANDLER_A 146
IDT_HANDLER_A 147
IDT_HANDLER_A 148
IDT_HANDLER_A 149
IDT_HANDLER_A 150
IDT_HANDLER_A 151
IDT_HANDLER_A 152
IDT_HANDLER_A 153
IDT_HANDLER_A 154
IDT_HANDLER_A 155
IDT_HANDLER_A 156
IDT_HANDLER_A 157
IDT_HANDLER_A 158
IDT_HANDLER_A 159
IDT_HANDLER_A 160
IDT_HANDLER_A 161
IDT_HANDLER_A 162
IDT_HANDLER_A 163
IDT_HANDLER_A 164
IDT_HANDLER_A 165
IDT_HANDLER_A 166
IDT_HANDLER_A 167
IDT_HANDLER_A 168
IDT_HANDLER_A 169
IDT_HANDLER_A 170
IDT_HANDLER_A 171
IDT_HANDLER_A 172
IDT_HANDLER_A 173
IDT_HANDLER_A 174
IDT_HANDLER_A 175
IDT_HANDLER_A 176
IDT_HANDLER_A 177
IDT_HANDLER_A 178
IDT_HANDLER_A 179
IDT_HANDLER_A 180
IDT_HANDLER_A 181
IDT_HANDLER_A 182
IDT_HANDLER_A 183
IDT_HANDLER_A 184
IDT_HANDLER_A 185
IDT_HANDLER_A 186
IDT_HANDLER_A 187
IDT_HANDLER_A 188
IDT_HANDLER_A 189
IDT_HANDLER_A 190
IDT_HANDLER_A 191
IDT_HANDLER_A 192
IDT_HANDLER_A 193
IDT_HANDLER_A 194
IDT_HANDLER_A 195
IDT_HANDLER_A 196
IDT_HANDLER_A 197
IDT_HANDLER_A 198
IDT_HANDLER_A 199
IDT_HANDLER_A 200
IDT_HANDLER_A 201
IDT_HANDLER_A 202
IDT_HANDLER_A 203
IDT_HANDLER_A 204
IDT_HANDLER_A 205
IDT_HANDLER_A 206
IDT_HANDLER_A 207
IDT_HANDLER_A 208
IDT_HANDLER_A 209
IDT_HANDLER_A 210
IDT_HANDLER_A 211
IDT_HANDLER_A 212
IDT_HANDLER_A 213
IDT_HANDLER_A 214
IDT_HANDLER_A 215
IDT_HANDLER_A 216
IDT_HANDLER_A 217
IDT_HANDLER_A 218
IDT_HANDLER_A 219
IDT_HANDLER_A 220
IDT_HANDLER_A 221
IDT_HANDLER_A 222
IDT_HANDLER_A 223
IDT_HANDLER_A 224
IDT_HANDLER_A 225
IDT_HANDLER_A 226
IDT_HANDLER_A 227
IDT_HANDLER_A 228
IDT_HANDLER_A 229
IDT_HANDLER_A 230
IDT_HANDLER_A 231
IDT_HANDLER_A 232
IDT_HANDLER_A 233
IDT_HANDLER_A 234
IDT_HANDLER_A 235
IDT_HANDLER_A 236
IDT_HANDLER_A 237
IDT_HANDLER_A 238
IDT_HANDLER_A 239
IDT_HANDLER_A 240
IDT_HANDLER_A 241
IDT_HANDLER_A 242
IDT_HANDLER_A 243
IDT_HANDLER_A 244
IDT_HANDLER_A 245
IDT_HANDLER_A 246
IDT_HANDLER_A 247
IDT_HANDLER_A 248
IDT_HANDLER_A 249
IDT_HANDLER_A 250
IDT_HANDLER_A 251
IDT_HANDLER_A 252
IDT_HANDLER_A 253
IDT_HANDLER_A 254
IDT_HANDLER_A 255

