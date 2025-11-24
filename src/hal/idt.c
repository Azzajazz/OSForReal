typedef struct PACKED {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
} Gate_Descriptor;

typedef struct {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} Interrupt_Frame;

#define IDT_GATE_TASK 0x5
#define IDT_GATE_INT16 0x6
#define IDT_GATE_TRAP16 0x7
#define IDT_GATE_INT32 0xE
#define IDT_GATE_TRAP32 0xF

#define IDT_PRIV_1 (1 << 5)
#define IDT_PRIV_2 (2 << 5)
#define IDT_PRIV_3 (3 << 5)

#define IDT_PRESENT (1 << 7)



struct PACKED {
    uint16_t size;
    uint32_t offset;
} idtr;

Gate_Descriptor idt[256];

Gate_Descriptor idt_create_descriptor(
    uint32_t offset,
    uint16_t segment_selector,
    uint8_t flags
) {
    Gate_Descriptor descriptor;
    
    descriptor.offset_low = offset & 0xffff;
    descriptor.segment_selector = segment_selector;
    descriptor.reserved = 0;
    descriptor.flags = flags;
    descriptor.offset_high = (offset >> 16) & 0xffff;

    return descriptor;
}

void fmt_print_impl(char *fmt, ...); // @TODO: Temporary

#define EXCEPTION_STUB(vector) exception_stub_##vector

#define EXCEPTION_STUB_IMPL(vector) \
void INTERRUPT EXCEPTION_STUB(vector)(Interrupt_Frame *frame) { \
    UNUSED(frame); \
    fmt_print_impl("Interrupt: %d\n", vector, 0); \
}

#define EXCEPTION_STUB_ERR_CODE_IMPL(vector) \
void INTERRUPT EXCEPTION_STUB(vector)(Interrupt_Frame *frame, uint32_t error_code) { \
    UNUSED(frame); \
    UNUSED(error_code); \
    fmt_print_impl("Interrupt: %d\n", vector, 0); \
}


#define INTERRUPT_STUB(vector) interrupt_stub_##vector

#define INTERRUPT_STUB_IMPL(vector) \
void INTERRUPT INTERRUPT_STUB(vector)(Interrupt_Frame *frame) { \
    registered_handlers[vector - 32](frame); \
}

// A place for driver registered interrupts to live.
// Drivers currently cannot register interrupts for exceptions, hence why there are 256 - 32 interrupts.
typedef void (*Registered_Handler)(Interrupt_Frame *frame);
Registered_Handler registered_handlers[256 - 32];

void default_registered_handler(Interrupt_Frame *frame) {
    UNUSED(frame);
}

// Intel defined exceptions.
EXCEPTION_STUB_IMPL(0);
EXCEPTION_STUB_IMPL(1);
EXCEPTION_STUB_IMPL(2);
EXCEPTION_STUB_IMPL(3);
EXCEPTION_STUB_IMPL(4);
EXCEPTION_STUB_IMPL(5);
EXCEPTION_STUB_IMPL(6);
EXCEPTION_STUB_IMPL(7);
EXCEPTION_STUB_ERR_CODE_IMPL(8);
EXCEPTION_STUB_IMPL(9);
EXCEPTION_STUB_IMPL(10);
EXCEPTION_STUB_ERR_CODE_IMPL(11);
EXCEPTION_STUB_ERR_CODE_IMPL(12);
EXCEPTION_STUB_ERR_CODE_IMPL(13);
EXCEPTION_STUB_ERR_CODE_IMPL(14);
EXCEPTION_STUB_IMPL(15);
EXCEPTION_STUB_IMPL(16);
EXCEPTION_STUB_ERR_CODE_IMPL(17);
EXCEPTION_STUB_IMPL(18);
EXCEPTION_STUB_IMPL(19);
EXCEPTION_STUB_IMPL(20);
EXCEPTION_STUB_ERR_CODE_IMPL(21);
EXCEPTION_STUB_IMPL(22);
EXCEPTION_STUB_IMPL(23);
EXCEPTION_STUB_IMPL(24);
EXCEPTION_STUB_IMPL(25);
EXCEPTION_STUB_IMPL(26);
EXCEPTION_STUB_IMPL(27);
EXCEPTION_STUB_IMPL(28);
EXCEPTION_STUB_ERR_CODE_IMPL(29);
EXCEPTION_STUB_ERR_CODE_IMPL(30);
EXCEPTION_STUB_IMPL(31);
INTERRUPT_STUB_IMPL(32);
INTERRUPT_STUB_IMPL(33);
INTERRUPT_STUB_IMPL(34);
INTERRUPT_STUB_IMPL(35);
INTERRUPT_STUB_IMPL(36);
INTERRUPT_STUB_IMPL(37);
INTERRUPT_STUB_IMPL(38);
INTERRUPT_STUB_IMPL(39);
INTERRUPT_STUB_IMPL(40);
INTERRUPT_STUB_IMPL(41);
INTERRUPT_STUB_IMPL(42);
INTERRUPT_STUB_IMPL(43);
INTERRUPT_STUB_IMPL(44);
INTERRUPT_STUB_IMPL(45);
INTERRUPT_STUB_IMPL(46);
INTERRUPT_STUB_IMPL(47);
INTERRUPT_STUB_IMPL(48);
INTERRUPT_STUB_IMPL(49);
INTERRUPT_STUB_IMPL(50);
INTERRUPT_STUB_IMPL(51);
INTERRUPT_STUB_IMPL(52);
INTERRUPT_STUB_IMPL(53);
INTERRUPT_STUB_IMPL(54);
INTERRUPT_STUB_IMPL(55);
INTERRUPT_STUB_IMPL(56);
INTERRUPT_STUB_IMPL(57);
INTERRUPT_STUB_IMPL(58);
INTERRUPT_STUB_IMPL(59);
INTERRUPT_STUB_IMPL(60);
INTERRUPT_STUB_IMPL(61);
INTERRUPT_STUB_IMPL(62);
INTERRUPT_STUB_IMPL(63);
INTERRUPT_STUB_IMPL(64);
INTERRUPT_STUB_IMPL(65);
INTERRUPT_STUB_IMPL(66);
INTERRUPT_STUB_IMPL(67);
INTERRUPT_STUB_IMPL(68);
INTERRUPT_STUB_IMPL(69);
INTERRUPT_STUB_IMPL(70);
INTERRUPT_STUB_IMPL(71);
INTERRUPT_STUB_IMPL(72);
INTERRUPT_STUB_IMPL(73);
INTERRUPT_STUB_IMPL(74);
INTERRUPT_STUB_IMPL(75);
INTERRUPT_STUB_IMPL(76);
INTERRUPT_STUB_IMPL(77);
INTERRUPT_STUB_IMPL(78);
INTERRUPT_STUB_IMPL(79);
INTERRUPT_STUB_IMPL(80);
INTERRUPT_STUB_IMPL(81);
INTERRUPT_STUB_IMPL(82);
INTERRUPT_STUB_IMPL(83);
INTERRUPT_STUB_IMPL(84);
INTERRUPT_STUB_IMPL(85);
INTERRUPT_STUB_IMPL(86);
INTERRUPT_STUB_IMPL(87);
INTERRUPT_STUB_IMPL(88);
INTERRUPT_STUB_IMPL(89);
INTERRUPT_STUB_IMPL(90);
INTERRUPT_STUB_IMPL(91);
INTERRUPT_STUB_IMPL(92);
INTERRUPT_STUB_IMPL(93);
INTERRUPT_STUB_IMPL(94);
INTERRUPT_STUB_IMPL(95);
INTERRUPT_STUB_IMPL(96);
INTERRUPT_STUB_IMPL(97);
INTERRUPT_STUB_IMPL(98);
INTERRUPT_STUB_IMPL(99);
INTERRUPT_STUB_IMPL(100);
INTERRUPT_STUB_IMPL(101);
INTERRUPT_STUB_IMPL(102);
INTERRUPT_STUB_IMPL(103);
INTERRUPT_STUB_IMPL(104);
INTERRUPT_STUB_IMPL(105);
INTERRUPT_STUB_IMPL(106);
INTERRUPT_STUB_IMPL(107);
INTERRUPT_STUB_IMPL(108);
INTERRUPT_STUB_IMPL(109);
INTERRUPT_STUB_IMPL(110);
INTERRUPT_STUB_IMPL(111);
INTERRUPT_STUB_IMPL(112);
INTERRUPT_STUB_IMPL(113);
INTERRUPT_STUB_IMPL(114);
INTERRUPT_STUB_IMPL(115);
INTERRUPT_STUB_IMPL(116);
INTERRUPT_STUB_IMPL(117);
INTERRUPT_STUB_IMPL(118);
INTERRUPT_STUB_IMPL(119);
INTERRUPT_STUB_IMPL(120);
INTERRUPT_STUB_IMPL(121);
INTERRUPT_STUB_IMPL(122);
INTERRUPT_STUB_IMPL(123);
INTERRUPT_STUB_IMPL(124);
INTERRUPT_STUB_IMPL(125);
INTERRUPT_STUB_IMPL(126);
INTERRUPT_STUB_IMPL(127);
INTERRUPT_STUB_IMPL(128);
INTERRUPT_STUB_IMPL(129);
INTERRUPT_STUB_IMPL(130);
INTERRUPT_STUB_IMPL(131);
INTERRUPT_STUB_IMPL(132);
INTERRUPT_STUB_IMPL(133);
INTERRUPT_STUB_IMPL(134);
INTERRUPT_STUB_IMPL(135);
INTERRUPT_STUB_IMPL(136);
INTERRUPT_STUB_IMPL(137);
INTERRUPT_STUB_IMPL(138);
INTERRUPT_STUB_IMPL(139);
INTERRUPT_STUB_IMPL(140);
INTERRUPT_STUB_IMPL(141);
INTERRUPT_STUB_IMPL(142);
INTERRUPT_STUB_IMPL(143);
INTERRUPT_STUB_IMPL(144);
INTERRUPT_STUB_IMPL(145);
INTERRUPT_STUB_IMPL(146);
INTERRUPT_STUB_IMPL(147);
INTERRUPT_STUB_IMPL(148);
INTERRUPT_STUB_IMPL(149);
INTERRUPT_STUB_IMPL(150);
INTERRUPT_STUB_IMPL(151);
INTERRUPT_STUB_IMPL(152);
INTERRUPT_STUB_IMPL(153);
INTERRUPT_STUB_IMPL(154);
INTERRUPT_STUB_IMPL(155);
INTERRUPT_STUB_IMPL(156);
INTERRUPT_STUB_IMPL(157);
INTERRUPT_STUB_IMPL(158);
INTERRUPT_STUB_IMPL(159);
INTERRUPT_STUB_IMPL(160);
INTERRUPT_STUB_IMPL(161);
INTERRUPT_STUB_IMPL(162);
INTERRUPT_STUB_IMPL(163);
INTERRUPT_STUB_IMPL(164);
INTERRUPT_STUB_IMPL(165);
INTERRUPT_STUB_IMPL(166);
INTERRUPT_STUB_IMPL(167);
INTERRUPT_STUB_IMPL(168);
INTERRUPT_STUB_IMPL(169);
INTERRUPT_STUB_IMPL(170);
INTERRUPT_STUB_IMPL(171);
INTERRUPT_STUB_IMPL(172);
INTERRUPT_STUB_IMPL(173);
INTERRUPT_STUB_IMPL(174);
INTERRUPT_STUB_IMPL(175);
INTERRUPT_STUB_IMPL(176);
INTERRUPT_STUB_IMPL(177);
INTERRUPT_STUB_IMPL(178);
INTERRUPT_STUB_IMPL(179);
INTERRUPT_STUB_IMPL(180);
INTERRUPT_STUB_IMPL(181);
INTERRUPT_STUB_IMPL(182);
INTERRUPT_STUB_IMPL(183);
INTERRUPT_STUB_IMPL(184);
INTERRUPT_STUB_IMPL(185);
INTERRUPT_STUB_IMPL(186);
INTERRUPT_STUB_IMPL(187);
INTERRUPT_STUB_IMPL(188);
INTERRUPT_STUB_IMPL(189);
INTERRUPT_STUB_IMPL(190);
INTERRUPT_STUB_IMPL(191);
INTERRUPT_STUB_IMPL(192);
INTERRUPT_STUB_IMPL(193);
INTERRUPT_STUB_IMPL(194);
INTERRUPT_STUB_IMPL(195);
INTERRUPT_STUB_IMPL(196);
INTERRUPT_STUB_IMPL(197);
INTERRUPT_STUB_IMPL(198);
INTERRUPT_STUB_IMPL(199);
INTERRUPT_STUB_IMPL(200);
INTERRUPT_STUB_IMPL(201);
INTERRUPT_STUB_IMPL(202);
INTERRUPT_STUB_IMPL(203);
INTERRUPT_STUB_IMPL(204);
INTERRUPT_STUB_IMPL(205);
INTERRUPT_STUB_IMPL(206);
INTERRUPT_STUB_IMPL(207);
INTERRUPT_STUB_IMPL(208);
INTERRUPT_STUB_IMPL(209);
INTERRUPT_STUB_IMPL(210);
INTERRUPT_STUB_IMPL(211);
INTERRUPT_STUB_IMPL(212);
INTERRUPT_STUB_IMPL(213);
INTERRUPT_STUB_IMPL(214);
INTERRUPT_STUB_IMPL(215);
INTERRUPT_STUB_IMPL(216);
INTERRUPT_STUB_IMPL(217);
INTERRUPT_STUB_IMPL(218);
INTERRUPT_STUB_IMPL(219);
INTERRUPT_STUB_IMPL(220);
INTERRUPT_STUB_IMPL(221);
INTERRUPT_STUB_IMPL(222);
INTERRUPT_STUB_IMPL(223);
INTERRUPT_STUB_IMPL(224);
INTERRUPT_STUB_IMPL(225);
INTERRUPT_STUB_IMPL(226);
INTERRUPT_STUB_IMPL(227);
INTERRUPT_STUB_IMPL(228);
INTERRUPT_STUB_IMPL(229);
INTERRUPT_STUB_IMPL(230);
INTERRUPT_STUB_IMPL(231);
INTERRUPT_STUB_IMPL(232);
INTERRUPT_STUB_IMPL(233);
INTERRUPT_STUB_IMPL(234);
INTERRUPT_STUB_IMPL(235);
INTERRUPT_STUB_IMPL(236);
INTERRUPT_STUB_IMPL(237);
INTERRUPT_STUB_IMPL(238);
INTERRUPT_STUB_IMPL(239);
INTERRUPT_STUB_IMPL(240);
INTERRUPT_STUB_IMPL(241);
INTERRUPT_STUB_IMPL(242);
INTERRUPT_STUB_IMPL(243);
INTERRUPT_STUB_IMPL(244);
INTERRUPT_STUB_IMPL(245);
INTERRUPT_STUB_IMPL(246);
INTERRUPT_STUB_IMPL(247);
INTERRUPT_STUB_IMPL(248);
INTERRUPT_STUB_IMPL(249);
INTERRUPT_STUB_IMPL(250);
INTERRUPT_STUB_IMPL(251);
INTERRUPT_STUB_IMPL(252);
INTERRUPT_STUB_IMPL(253);
INTERRUPT_STUB_IMPL(254);
INTERRUPT_STUB_IMPL(255);




void idt_init() {
    // Set up the IDT handlers.
    // This is suuuuper ugly, but I don't really see another way of doing this.
    uint16_t segment_selector = 0x8;
    uint8_t flags = IDT_GATE_TRAP32 | IDT_PRESENT;
    
    idt[0] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(0),
        segment_selector,
        flags
    );

    idt[1] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(1),
        segment_selector,
        flags
    );

    idt[2] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(2),
        segment_selector,
        flags
    );

    idt[3] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(3),
        segment_selector,
        flags
    );

    idt[4] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(4),
        segment_selector,
        flags
    );

    idt[5] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(5),
        segment_selector,
        flags
    );
    
    idt[6] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(6),
        segment_selector,
        flags
    );

    idt[7] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(7),
        segment_selector,
        flags
    );

    idt[8] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(8),
        segment_selector,
        flags
    );
    
    idt[9] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(9),
        segment_selector,
        flags
    );

    idt[10] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(10),
        segment_selector,
        flags
    );

    idt[11] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(11),
        segment_selector,
        flags
    );

    idt[12] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(12),
        segment_selector,
        flags
    );

    idt[13] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(13),
        segment_selector,
        flags
    );

    idt[14] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(14),
        segment_selector,
        flags
    );

    idt[15] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(15),
        segment_selector,
        flags
    );

    idt[16] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(16),
        segment_selector,
        flags
    );

    idt[17] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(17),
        segment_selector,
        flags
    );

    idt[18] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(18),
        segment_selector,
        flags
    );

    idt[19] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(19),
        segment_selector,
        flags
    );

    idt[20] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(20),
        segment_selector,
        flags
    );

    idt[21] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(21),
        segment_selector,
        flags
    );

    idt[22] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(22),
        segment_selector,
        flags
    );

    idt[23] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(23),
        segment_selector,
        flags
    );

    idt[24] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(24),
        segment_selector,
        flags
    );

    idt[25] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(25),
        segment_selector,
        flags
    );

    idt[26] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(26),
        segment_selector,
        flags
    );

    idt[27] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(27),
        segment_selector,
        flags
    );

    idt[28] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(28),
        segment_selector,
        flags
    );

    idt[29] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(29),
        segment_selector,
        flags
    );

    idt[30] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(30),
        segment_selector,
        flags
    );

    idt[31] = idt_create_descriptor(
        (uint32_t)EXCEPTION_STUB(31),
        segment_selector,
        flags
    );

    flags = IDT_GATE_INT32 | IDT_PRESENT;

    idt[32] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(32),
        segment_selector,
        flags
    );

    idt[33] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(33),
        segment_selector,
        flags
    );

    idt[34] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(34),
        segment_selector,
        flags
    );

    idt[35] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(35),
        segment_selector,
        flags
    );

    idt[36] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(36),
        segment_selector,
        flags
    );

    idt[37] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(37),
        segment_selector,
        flags
    );

    idt[38] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(38),
        segment_selector,
        flags
    );

    idt[39] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(39),
        segment_selector,
        flags
    );

    idt[40] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(40),
        segment_selector,
        flags
    );

    idt[41] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(41),
        segment_selector,
        flags
    );

    idt[42] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(42),
        segment_selector,
        flags
    );

    idt[43] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(43),
        segment_selector,
        flags
    );

    idt[44] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(44),
        segment_selector,
        flags
    );

    idt[45] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(45),
        segment_selector,
        flags
    );

    idt[46] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(46),
        segment_selector,
        flags
    );

    idt[47] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(47),
        segment_selector,
        flags
    );

    idt[48] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(48),
        segment_selector,
        flags
    );

    idt[49] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(49),
        segment_selector,
        flags
    );

    idt[50] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(50),
        segment_selector,
        flags
    );

    idt[51] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(51),
        segment_selector,
        flags
    );

    idt[52] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(52),
        segment_selector,
        flags
    );

    idt[53] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(53),
        segment_selector,
        flags
    );

    idt[54] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(54),
        segment_selector,
        flags
    );

    idt[55] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(55),
        segment_selector,
        flags
    );

    idt[56] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(56),
        segment_selector,
        flags
    );

    idt[57] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(57),
        segment_selector,
        flags
    );

    idt[58] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(58),
        segment_selector,
        flags
    );

    idt[59] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(59),
        segment_selector,
        flags
    );

    idt[60] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(60),
        segment_selector,
        flags
    );

    idt[61] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(61),
        segment_selector,
        flags
    );

    idt[62] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(62),
        segment_selector,
        flags
    );

    idt[63] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(63),
        segment_selector,
        flags
    );

    idt[64] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(64),
        segment_selector,
        flags
    );

    idt[65] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(65),
        segment_selector,
        flags
    );

    idt[66] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(66),
        segment_selector,
        flags
    );

    idt[67] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(67),
        segment_selector,
        flags
    );

    idt[68] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(68),
        segment_selector,
        flags
    );

    idt[69] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(69),
        segment_selector,
        flags
    );

    idt[70] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(70),
        segment_selector,
        flags
    );

    idt[71] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(71),
        segment_selector,
        flags
    );

    idt[72] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(72),
        segment_selector,
        flags
    );

    idt[73] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(73),
        segment_selector,
        flags
    );

    idt[74] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(74),
        segment_selector,
        flags
    );

    idt[75] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(75),
        segment_selector,
        flags
    );

    idt[76] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(76),
        segment_selector,
        flags
    );

    idt[77] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(77),
        segment_selector,
        flags
    );

    idt[78] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(78),
        segment_selector,
        flags
    );

    idt[79] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(79),
        segment_selector,
        flags
    );

    idt[80] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(80),
        segment_selector,
        flags
    );

    idt[81] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(81),
        segment_selector,
        flags
    );

    idt[82] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(82),
        segment_selector,
        flags
    );

    idt[83] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(83),
        segment_selector,
        flags
    );

    idt[84] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(84),
        segment_selector,
        flags
    );

    idt[85] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(85),
        segment_selector,
        flags
    );

    idt[86] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(86),
        segment_selector,
        flags
    );

    idt[87] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(87),
        segment_selector,
        flags
    );

    idt[88] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(88),
        segment_selector,
        flags
    );

    idt[89] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(89),
        segment_selector,
        flags
    );

    idt[90] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(90),
        segment_selector,
        flags
    );

    idt[91] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(91),
        segment_selector,
        flags
    );

    idt[92] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(92),
        segment_selector,
        flags
    );

    idt[93] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(93),
        segment_selector,
        flags
    );

    idt[94] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(94),
        segment_selector,
        flags
    );

    idt[95] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(95),
        segment_selector,
        flags
    );

    idt[96] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(96),
        segment_selector,
        flags
    );

    idt[97] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(97),
        segment_selector,
        flags
    );

    idt[98] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(98),
        segment_selector,
        flags
    );

    idt[99] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(99),
        segment_selector,
        flags
    );

    idt[100] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(100),
        segment_selector,
        flags
    );

    idt[101] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(101),
        segment_selector,
        flags
    );

    idt[102] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(102),
        segment_selector,
        flags
    );

    idt[103] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(103),
        segment_selector,
        flags
    );

    idt[104] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(104),
        segment_selector,
        flags
    );

    idt[105] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(105),
        segment_selector,
        flags
    );

    idt[106] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(106),
        segment_selector,
        flags
    );

    idt[107] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(107),
        segment_selector,
        flags
    );

    idt[108] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(108),
        segment_selector,
        flags
    );

    idt[109] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(109),
        segment_selector,
        flags
    );

    idt[110] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(110),
        segment_selector,
        flags
    );

    idt[111] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(111),
        segment_selector,
        flags
    );

    idt[112] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(112),
        segment_selector,
        flags
    );

    idt[113] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(113),
        segment_selector,
        flags
    );

    idt[114] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(114),
        segment_selector,
        flags
    );

    idt[115] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(115),
        segment_selector,
        flags
    );

    idt[116] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(116),
        segment_selector,
        flags
    );

    idt[117] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(117),
        segment_selector,
        flags
    );

    idt[118] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(118),
        segment_selector,
        flags
    );

    idt[119] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(119),
        segment_selector,
        flags
    );

    idt[120] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(120),
        segment_selector,
        flags
    );

    idt[121] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(121),
        segment_selector,
        flags
    );

    idt[122] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(122),
        segment_selector,
        flags
    );

    idt[123] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(123),
        segment_selector,
        flags
    );

    idt[124] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(124),
        segment_selector,
        flags
    );

    idt[125] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(125),
        segment_selector,
        flags
    );

    idt[126] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(126),
        segment_selector,
        flags
    );

    idt[127] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(127),
        segment_selector,
        flags
    );

    idt[128] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(128),
        segment_selector,
        flags
    );

    idt[129] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(129),
        segment_selector,
        flags
    );

    idt[130] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(130),
        segment_selector,
        flags
    );

    idt[131] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(131),
        segment_selector,
        flags
    );

    idt[132] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(132),
        segment_selector,
        flags
    );

    idt[133] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(133),
        segment_selector,
        flags
    );

    idt[134] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(134),
        segment_selector,
        flags
    );

    idt[135] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(135),
        segment_selector,
        flags
    );

    idt[136] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(136),
        segment_selector,
        flags
    );

    idt[137] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(137),
        segment_selector,
        flags
    );

    idt[138] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(138),
        segment_selector,
        flags
    );

    idt[139] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(139),
        segment_selector,
        flags
    );

    idt[140] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(140),
        segment_selector,
        flags
    );

    idt[141] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(141),
        segment_selector,
        flags
    );

    idt[142] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(142),
        segment_selector,
        flags
    );

    idt[143] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(143),
        segment_selector,
        flags
    );

    idt[144] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(144),
        segment_selector,
        flags
    );

    idt[145] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(145),
        segment_selector,
        flags
    );

    idt[146] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(146),
        segment_selector,
        flags
    );

    idt[147] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(147),
        segment_selector,
        flags
    );

    idt[148] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(148),
        segment_selector,
        flags
    );

    idt[149] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(149),
        segment_selector,
        flags
    );

    idt[150] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(150),
        segment_selector,
        flags
    );

    idt[151] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(151),
        segment_selector,
        flags
    );

    idt[152] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(152),
        segment_selector,
        flags
    );

    idt[153] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(153),
        segment_selector,
        flags
    );

    idt[154] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(154),
        segment_selector,
        flags
    );

    idt[155] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(155),
        segment_selector,
        flags
    );

    idt[156] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(156),
        segment_selector,
        flags
    );

    idt[157] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(157),
        segment_selector,
        flags
    );

    idt[158] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(158),
        segment_selector,
        flags
    );

    idt[159] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(159),
        segment_selector,
        flags
    );

    idt[160] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(160),
        segment_selector,
        flags
    );

    idt[161] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(161),
        segment_selector,
        flags
    );

    idt[162] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(162),
        segment_selector,
        flags
    );

    idt[163] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(163),
        segment_selector,
        flags
    );

    idt[164] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(164),
        segment_selector,
        flags
    );

    idt[165] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(165),
        segment_selector,
        flags
    );

    idt[166] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(166),
        segment_selector,
        flags
    );

    idt[167] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(167),
        segment_selector,
        flags
    );

    idt[168] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(168),
        segment_selector,
        flags
    );

    idt[169] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(169),
        segment_selector,
        flags
    );

    idt[170] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(170),
        segment_selector,
        flags
    );

    idt[171] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(171),
        segment_selector,
        flags
    );

    idt[172] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(172),
        segment_selector,
        flags
    );

    idt[173] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(173),
        segment_selector,
        flags
    );

    idt[174] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(174),
        segment_selector,
        flags
    );

    idt[175] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(175),
        segment_selector,
        flags
    );

    idt[176] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(176),
        segment_selector,
        flags
    );

    idt[177] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(177),
        segment_selector,
        flags
    );

    idt[178] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(178),
        segment_selector,
        flags
    );

    idt[179] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(179),
        segment_selector,
        flags
    );

    idt[180] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(180),
        segment_selector,
        flags
    );

    idt[181] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(181),
        segment_selector,
        flags
    );

    idt[182] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(182),
        segment_selector,
        flags
    );

    idt[183] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(183),
        segment_selector,
        flags
    );

    idt[184] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(184),
        segment_selector,
        flags
    );

    idt[185] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(185),
        segment_selector,
        flags
    );

    idt[186] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(186),
        segment_selector,
        flags
    );

    idt[187] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(187),
        segment_selector,
        flags
    );

    idt[188] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(188),
        segment_selector,
        flags
    );

    idt[189] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(189),
        segment_selector,
        flags
    );

    idt[190] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(190),
        segment_selector,
        flags
    );

    idt[191] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(191),
        segment_selector,
        flags
    );

    idt[192] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(192),
        segment_selector,
        flags
    );

    idt[193] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(193),
        segment_selector,
        flags
    );

    idt[194] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(194),
        segment_selector,
        flags
    );

    idt[195] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(195),
        segment_selector,
        flags
    );

    idt[196] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(196),
        segment_selector,
        flags
    );

    idt[197] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(197),
        segment_selector,
        flags
    );

    idt[198] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(198),
        segment_selector,
        flags
    );

    idt[199] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(199),
        segment_selector,
        flags
    );

    idt[200] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(200),
        segment_selector,
        flags
    );

    idt[201] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(201),
        segment_selector,
        flags
    );

    idt[202] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(202),
        segment_selector,
        flags
    );

    idt[203] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(203),
        segment_selector,
        flags
    );

    idt[204] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(204),
        segment_selector,
        flags
    );

    idt[205] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(205),
        segment_selector,
        flags
    );

    idt[206] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(206),
        segment_selector,
        flags
    );

    idt[207] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(207),
        segment_selector,
        flags
    );

    idt[208] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(208),
        segment_selector,
        flags
    );

    idt[209] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(209),
        segment_selector,
        flags
    );

    idt[210] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(210),
        segment_selector,
        flags
    );

    idt[211] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(211),
        segment_selector,
        flags
    );

    idt[212] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(212),
        segment_selector,
        flags
    );

    idt[213] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(213),
        segment_selector,
        flags
    );

    idt[214] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(214),
        segment_selector,
        flags
    );

    idt[215] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(215),
        segment_selector,
        flags
    );

    idt[216] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(216),
        segment_selector,
        flags
    );

    idt[217] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(217),
        segment_selector,
        flags
    );

    idt[218] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(218),
        segment_selector,
        flags
    );

    idt[219] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(219),
        segment_selector,
        flags
    );

    idt[220] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(220),
        segment_selector,
        flags
    );

    idt[221] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(221),
        segment_selector,
        flags
    );

    idt[222] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(222),
        segment_selector,
        flags
    );

    idt[223] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(223),
        segment_selector,
        flags
    );

    idt[224] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(224),
        segment_selector,
        flags
    );

    idt[225] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(225),
        segment_selector,
        flags
    );

    idt[226] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(226),
        segment_selector,
        flags
    );

    idt[227] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(227),
        segment_selector,
        flags
    );

    idt[228] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(228),
        segment_selector,
        flags
    );

    idt[229] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(229),
        segment_selector,
        flags
    );

    idt[230] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(230),
        segment_selector,
        flags
    );

    idt[231] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(231),
        segment_selector,
        flags
    );

    idt[232] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(232),
        segment_selector,
        flags
    );

    idt[233] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(233),
        segment_selector,
        flags
    );

    idt[234] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(234),
        segment_selector,
        flags
    );

    idt[235] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(235),
        segment_selector,
        flags
    );

    idt[236] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(236),
        segment_selector,
        flags
    );

    idt[237] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(237),
        segment_selector,
        flags
    );

    idt[238] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(238),
        segment_selector,
        flags
    );

    idt[239] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(239),
        segment_selector,
        flags
    );

    idt[240] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(240),
        segment_selector,
        flags
    );

    idt[241] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(241),
        segment_selector,
        flags
    );

    idt[242] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(242),
        segment_selector,
        flags
    );

    idt[243] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(243),
        segment_selector,
        flags
    );

    idt[244] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(244),
        segment_selector,
        flags
    );

    idt[245] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(245),
        segment_selector,
        flags
    );

    idt[246] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(246),
        segment_selector,
        flags
    );

    idt[247] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(247),
        segment_selector,
        flags
    );

    idt[248] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(248),
        segment_selector,
        flags
    );

    idt[249] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(249),
        segment_selector,
        flags
    );

    idt[250] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(250),
        segment_selector,
        flags
    );

    idt[251] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(251),
        segment_selector,
        flags
    );

    idt[252] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(252),
        segment_selector,
        flags
    );

    idt[253] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(253),
        segment_selector,
        flags
    );

    idt[254] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(254),
        segment_selector,
        flags
    );

    idt[255] = idt_create_descriptor(
        (uint32_t)INTERRUPT_STUB(255),
        segment_selector,
        flags
    );

    // Initialise the IDTR.
    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint32_t)idt;

    // Initialize the registered interrupts.
    for (size_t i = 0; i < ARRAY_LEN(registered_handlers); i++) {
        registered_handlers[i] = default_registered_handler;
    }

    // Load the IDT and enable interrupts.
    asm (
        "lidt [%0]\n\t"
        "sti\n\t"
        :
        : "m" (idtr)
    );
}

// @FIXME: Name.
void idt_register_handler(int vector, Registered_Handler handler) {
    // @FIXME: This should probably be an atomic load.
    registered_handlers[vector - 32] = handler;
}
