#include <Common.h>
#include <DescTabs.h>
#include <string.h>

static struct IDTEntry idt_entries[256] = { 0 };

static struct DescPointer idt_pointer = { 0 };

static uint64_t idt_bitmap[4] = { 0 };

uintptr_t idt_handlers[256] = { 0 };

uint64_t idt_args[256] = { 0 };


uint8_t IDTEntryAlloc(uint8_t attr, uint64_t arg, void (*handler)(struct Registers*, uint64_t))
{
	for(int i = 0; i < 4; i++) {
		if(idt_bitmap[i] == 0) continue;

		uint8_t bit = __builtin_ffsll(idt_bitmap[i]) - 1;

		uint8_t index = i * 64 + bit;

		IDTEntrySet(index, attr, arg, handler);

		return index;
	}

	return 0;
}

void IDTEntryFree(uint8_t vector)
{
	if((idt_bitmap[vector / 64] & (1ULL << (vector % 64))) != 0)
		Panic(NULL, "Can't free unused IDT entry");

	idt_bitmap[vector / 64] |= 1ULL << (vector % 64);

	idt_entries[vector].flags &= ~IDT_ATTR_PRESENT;
}

void IDTEntrySet(uint8_t vector, uint8_t attr, uint64_t arg, void (*hand)(struct Registers*, uint64_t))
{
	if((idt_bitmap[vector / 64] & (1ULL << (vector % 64))) == 0)
		Panic(NULL, "Tried to change already used IDT entry");

	idt_bitmap[vector / 64] &= ~(1ULL << (vector % 64));

	idt_entries[vector].flags = attr;

	idt_handlers[vector] = (uintptr_t) hand;
	idt_args[vector]     = arg;
}

static void IDTDefaultHandler(struct Registers *regs, uint64_t arg)
{
	Panic(regs, "Unhandled interrupt %l", regs->vector);
}

static void IDTEntryCreate(uint8_t vector, uint8_t attr, void (*handler)())
{
	struct IDTEntry *entry = &idt_entries[vector];

	entry->off_low  = ((uintptr_t) handler & 0x000000000000FFFF);
	entry->off_mid  = ((uintptr_t) handler & 0x00000000FFFF0000) >> 16;
	entry->off_high = ((uintptr_t) handler & 0xFFFFFFFF00000000) >> 32;

	entry->flags    = attr;
	entry->selector = 0x08;

	idt_handlers[vector] = (uintptr_t) IDTDefaultHandler;
}

void IDTClean()
{
	memset(idt_entries,  0, sizeof(idt_entries));
	memset(&idt_pointer, 0, sizeof(idt_pointer));
	memset(idt_bitmap,  -1, sizeof(idt_bitmap));
}

void IDTInstall()
{
	idt_pointer.size = sizeof(idt_entries) - 1;
	idt_pointer.addr = (uintptr_t) idt_entries;

	asm volatile("lidt %0" :: "m"(idt_pointer));
}

void IDTLoad()
{
	IDTClean();

	IDTEntryCreate(0, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler0);
	IDTEntryCreate(1, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler1);
	IDTEntryCreate(2, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler2);
	IDTEntryCreate(3, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler3);
	IDTEntryCreate(4, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler4);
	IDTEntryCreate(5, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler5);
	IDTEntryCreate(6, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler6);
	IDTEntryCreate(7, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler7);
	IDTEntryCreate(8, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler8);
	IDTEntryCreate(9, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler9);
	IDTEntryCreate(10, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler10);
	IDTEntryCreate(11, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler11);
	IDTEntryCreate(12, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler12);
	IDTEntryCreate(13, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler13);
	IDTEntryCreate(14, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler14);
	IDTEntryCreate(15, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler15);
	IDTEntryCreate(16, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler16);
	IDTEntryCreate(17, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler17);
	IDTEntryCreate(18, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler18);
	IDTEntryCreate(19, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler19);
	IDTEntryCreate(20, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler20);
	IDTEntryCreate(21, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler21);
	IDTEntryCreate(22, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler22);
	IDTEntryCreate(23, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler23);
	IDTEntryCreate(24, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler24);
	IDTEntryCreate(25, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler25);
	IDTEntryCreate(26, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler26);
	IDTEntryCreate(27, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler27);
	IDTEntryCreate(28, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler28);
	IDTEntryCreate(29, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler29);
	IDTEntryCreate(30, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler30);
	IDTEntryCreate(31, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler31);
	IDTEntryCreate(32, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler32);
	IDTEntryCreate(33, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler33);
	IDTEntryCreate(34, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler34);
	IDTEntryCreate(35, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler35);
	IDTEntryCreate(36, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler36);
	IDTEntryCreate(37, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler37);
	IDTEntryCreate(38, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler38);
	IDTEntryCreate(39, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler39);
	IDTEntryCreate(40, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler40);
	IDTEntryCreate(41, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler41);
	IDTEntryCreate(42, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler42);
	IDTEntryCreate(43, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler43);
	IDTEntryCreate(44, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler44);
	IDTEntryCreate(45, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler45);
	IDTEntryCreate(46, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler46);
	IDTEntryCreate(47, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler47);
	IDTEntryCreate(48, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler48);
	IDTEntryCreate(49, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler49);
	IDTEntryCreate(50, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler50);
	IDTEntryCreate(51, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler51);
	IDTEntryCreate(52, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler52);
	IDTEntryCreate(53, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler53);
	IDTEntryCreate(54, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler54);
	IDTEntryCreate(55, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler55);
	IDTEntryCreate(56, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler56);
	IDTEntryCreate(57, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler57);
	IDTEntryCreate(58, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler58);
	IDTEntryCreate(59, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler59);
	IDTEntryCreate(60, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler60);
	IDTEntryCreate(61, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler61);
	IDTEntryCreate(62, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler62);
	IDTEntryCreate(63, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler63);
	IDTEntryCreate(64, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler64);
	IDTEntryCreate(65, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler65);
	IDTEntryCreate(66, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler66);
	IDTEntryCreate(67, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler67);
	IDTEntryCreate(68, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler68);
	IDTEntryCreate(69, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler69);
	IDTEntryCreate(70, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler70);
	IDTEntryCreate(71, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler71);
	IDTEntryCreate(72, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler72);
	IDTEntryCreate(73, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler73);
	IDTEntryCreate(74, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler74);
	IDTEntryCreate(75, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler75);
	IDTEntryCreate(76, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler76);
	IDTEntryCreate(77, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler77);
	IDTEntryCreate(78, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler78);
	IDTEntryCreate(79, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler79);
	IDTEntryCreate(80, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler80);
	IDTEntryCreate(81, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler81);
	IDTEntryCreate(82, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler82);
	IDTEntryCreate(83, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler83);
	IDTEntryCreate(84, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler84);
	IDTEntryCreate(85, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler85);
	IDTEntryCreate(86, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler86);
	IDTEntryCreate(87, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler87);
	IDTEntryCreate(88, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler88);
	IDTEntryCreate(89, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler89);
	IDTEntryCreate(90, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler90);
	IDTEntryCreate(91, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler91);
	IDTEntryCreate(92, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler92);
	IDTEntryCreate(93, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler93);
	IDTEntryCreate(94, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler94);
	IDTEntryCreate(95, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler95);
	IDTEntryCreate(96, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler96);
	IDTEntryCreate(97, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler97);
	IDTEntryCreate(98, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler98);
	IDTEntryCreate(99, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler99);
	IDTEntryCreate(100, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler100);
	IDTEntryCreate(101, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler101);
	IDTEntryCreate(102, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler102);
	IDTEntryCreate(103, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler103);
	IDTEntryCreate(104, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler104);
	IDTEntryCreate(105, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler105);
	IDTEntryCreate(106, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler106);
	IDTEntryCreate(107, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler107);
	IDTEntryCreate(108, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler108);
	IDTEntryCreate(109, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler109);
	IDTEntryCreate(110, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler110);
	IDTEntryCreate(111, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler111);
	IDTEntryCreate(112, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler112);
	IDTEntryCreate(113, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler113);
	IDTEntryCreate(114, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler114);
	IDTEntryCreate(115, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler115);
	IDTEntryCreate(116, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler116);
	IDTEntryCreate(117, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler117);
	IDTEntryCreate(118, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler118);
	IDTEntryCreate(119, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler119);
	IDTEntryCreate(120, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler120);
	IDTEntryCreate(121, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler121);
	IDTEntryCreate(122, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler122);
	IDTEntryCreate(123, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler123);
	IDTEntryCreate(124, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler124);
	IDTEntryCreate(125, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler125);
	IDTEntryCreate(126, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler126);
	IDTEntryCreate(127, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler127);
	IDTEntryCreate(128, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler128);
	IDTEntryCreate(129, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler129);
	IDTEntryCreate(130, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler130);
	IDTEntryCreate(131, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler131);
	IDTEntryCreate(132, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler132);
	IDTEntryCreate(133, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler133);
	IDTEntryCreate(134, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler134);
	IDTEntryCreate(135, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler135);
	IDTEntryCreate(136, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler136);
	IDTEntryCreate(137, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler137);
	IDTEntryCreate(138, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler138);
	IDTEntryCreate(139, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler139);
	IDTEntryCreate(140, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler140);
	IDTEntryCreate(141, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler141);
	IDTEntryCreate(142, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler142);
	IDTEntryCreate(143, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler143);
	IDTEntryCreate(144, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler144);
	IDTEntryCreate(145, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler145);
	IDTEntryCreate(146, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler146);
	IDTEntryCreate(147, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler147);
	IDTEntryCreate(148, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler148);
	IDTEntryCreate(149, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler149);
	IDTEntryCreate(150, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler150);
	IDTEntryCreate(151, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler151);
	IDTEntryCreate(152, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler152);
	IDTEntryCreate(153, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler153);
	IDTEntryCreate(154, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler154);
	IDTEntryCreate(155, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler155);
	IDTEntryCreate(156, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler156);
	IDTEntryCreate(157, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler157);
	IDTEntryCreate(158, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler158);
	IDTEntryCreate(159, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler159);
	IDTEntryCreate(160, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler160);
	IDTEntryCreate(161, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler161);
	IDTEntryCreate(162, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler162);
	IDTEntryCreate(163, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler163);
	IDTEntryCreate(164, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler164);
	IDTEntryCreate(165, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler165);
	IDTEntryCreate(166, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler166);
	IDTEntryCreate(167, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler167);
	IDTEntryCreate(168, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler168);
	IDTEntryCreate(169, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler169);
	IDTEntryCreate(170, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler170);
	IDTEntryCreate(171, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler171);
	IDTEntryCreate(172, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler172);
	IDTEntryCreate(173, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler173);
	IDTEntryCreate(174, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler174);
	IDTEntryCreate(175, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler175);
	IDTEntryCreate(176, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler176);
	IDTEntryCreate(177, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler177);
	IDTEntryCreate(178, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler178);
	IDTEntryCreate(179, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler179);
	IDTEntryCreate(180, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler180);
	IDTEntryCreate(181, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler181);
	IDTEntryCreate(182, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler182);
	IDTEntryCreate(183, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler183);
	IDTEntryCreate(184, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler184);
	IDTEntryCreate(185, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler185);
	IDTEntryCreate(186, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler186);
	IDTEntryCreate(187, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler187);
	IDTEntryCreate(188, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler188);
	IDTEntryCreate(189, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler189);
	IDTEntryCreate(190, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler190);
	IDTEntryCreate(191, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler191);
	IDTEntryCreate(192, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler192);
	IDTEntryCreate(193, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler193);
	IDTEntryCreate(194, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler194);
	IDTEntryCreate(195, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler195);
	IDTEntryCreate(196, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler196);
	IDTEntryCreate(197, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler197);
	IDTEntryCreate(198, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler198);
	IDTEntryCreate(199, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler199);
	IDTEntryCreate(200, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler200);
	IDTEntryCreate(201, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler201);
	IDTEntryCreate(202, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler202);
	IDTEntryCreate(203, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler203);
	IDTEntryCreate(204, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler204);
	IDTEntryCreate(205, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler205);
	IDTEntryCreate(206, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler206);
	IDTEntryCreate(207, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler207);
	IDTEntryCreate(208, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler208);
	IDTEntryCreate(209, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler209);
	IDTEntryCreate(210, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler210);
	IDTEntryCreate(211, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler211);
	IDTEntryCreate(212, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler212);
	IDTEntryCreate(213, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler213);
	IDTEntryCreate(214, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler214);
	IDTEntryCreate(215, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler215);
	IDTEntryCreate(216, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler216);
	IDTEntryCreate(217, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler217);
	IDTEntryCreate(218, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler218);
	IDTEntryCreate(219, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler219);
	IDTEntryCreate(220, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler220);
	IDTEntryCreate(221, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler221);
	IDTEntryCreate(222, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler222);
	IDTEntryCreate(223, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler223);
	IDTEntryCreate(224, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler224);
	IDTEntryCreate(225, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler225);
	IDTEntryCreate(226, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler226);
	IDTEntryCreate(227, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler227);
	IDTEntryCreate(228, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler228);
	IDTEntryCreate(229, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler229);
	IDTEntryCreate(230, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler230);
	IDTEntryCreate(231, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler231);
	IDTEntryCreate(232, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler232);
	IDTEntryCreate(233, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler233);
	IDTEntryCreate(234, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler234);
	IDTEntryCreate(235, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler235);
	IDTEntryCreate(236, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler236);
	IDTEntryCreate(237, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler237);
	IDTEntryCreate(238, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler238);
	IDTEntryCreate(239, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler239);
	IDTEntryCreate(240, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler240);
	IDTEntryCreate(241, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler241);
	IDTEntryCreate(242, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler242);
	IDTEntryCreate(243, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler243);
	IDTEntryCreate(244, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler244);
	IDTEntryCreate(245, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler245);
	IDTEntryCreate(246, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler246);
	IDTEntryCreate(247, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler247);
	IDTEntryCreate(248, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler248);
	IDTEntryCreate(249, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler249);
	IDTEntryCreate(250, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler250);
	IDTEntryCreate(251, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler251);
	IDTEntryCreate(252, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler252);
	IDTEntryCreate(253, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler253);
	IDTEntryCreate(254, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler254);
	IDTEntryCreate(255, IDT_ATTR_PRESENT | IDT_ATTR_TRAP, IDTCommonHandler255);

	ExceptionsLoad();

	

	IDTInstall();
}
