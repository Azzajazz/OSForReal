Gate_Descriptor idt[256];

Gate_Descriptor interrupts_create_descriptor(
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

void fmt_print_impl(char *fmt, ...);

// A place for driver registered interrupts to live.
// Drivers currently cannot register interrupts for exceptions, hence why there are 256 - 32 interrupts.
Registered_Handler registered_handlers[256 - 32];

void default_registered_handler(Interrupt_Frame *frame) {
    UNUSED(frame);
}

// Declarations of interrupt stubs.
void exc_start_0(void);
void exc_start_1(void);
void exc_start_2(void);
void exc_start_3(void);
void exc_start_4(void);
void exc_start_5(void);
void exc_start_6(void);
void exc_start_7(void);
void exc_start_8(void);
void exc_start_9(void);
void exc_start_10(void);
void exc_start_11(void);
void exc_start_12(void);
void exc_start_13(void);
void exc_start_14(void);
void exc_start_15(void);
void exc_start_16(void);
void exc_start_17(void);
void exc_start_18(void);
void exc_start_19(void);
void exc_start_20(void);
void exc_start_21(void);
void exc_start_22(void);
void exc_start_23(void);
void exc_start_24(void);
void exc_start_25(void);
void exc_start_26(void);
void exc_start_27(void);
void exc_start_28(void);
void exc_start_29(void);
void exc_start_30(void);
void exc_start_31(void);
void int_start_32(void);
void int_start_33(void);
void int_start_34(void);
void int_start_35(void);
void int_start_36(void);
void int_start_37(void);
void int_start_38(void);
void int_start_39(void);
void int_start_40(void);
void int_start_41(void);
void int_start_42(void);
void int_start_43(void);
void int_start_44(void);
void int_start_45(void);
void int_start_46(void);
void int_start_47(void);
void int_start_48(void);
void int_start_49(void);
void int_start_50(void);
void int_start_51(void);
void int_start_52(void);
void int_start_53(void);
void int_start_54(void);
void int_start_55(void);
void int_start_56(void);
void int_start_57(void);
void int_start_58(void);
void int_start_59(void);
void int_start_60(void);
void int_start_61(void);
void int_start_62(void);
void int_start_63(void);
void int_start_64(void);
void int_start_65(void);
void int_start_66(void);
void int_start_67(void);
void int_start_68(void);
void int_start_69(void);
void int_start_70(void);
void int_start_71(void);
void int_start_72(void);
void int_start_73(void);
void int_start_74(void);
void int_start_75(void);
void int_start_76(void);
void int_start_77(void);
void int_start_78(void);
void int_start_79(void);
void int_start_80(void);
void int_start_81(void);
void int_start_82(void);
void int_start_83(void);
void int_start_84(void);
void int_start_85(void);
void int_start_86(void);
void int_start_87(void);
void int_start_88(void);
void int_start_89(void);
void int_start_90(void);
void int_start_91(void);
void int_start_92(void);
void int_start_93(void);
void int_start_94(void);
void int_start_95(void);
void int_start_96(void);
void int_start_97(void);
void int_start_98(void);
void int_start_99(void);
void int_start_100(void);
void int_start_101(void);
void int_start_102(void);
void int_start_103(void);
void int_start_104(void);
void int_start_105(void);
void int_start_106(void);
void int_start_107(void);
void int_start_108(void);
void int_start_109(void);
void int_start_110(void);
void int_start_111(void);
void int_start_112(void);
void int_start_113(void);
void int_start_114(void);
void int_start_115(void);
void int_start_116(void);
void int_start_117(void);
void int_start_118(void);
void int_start_119(void);
void int_start_120(void);
void int_start_121(void);
void int_start_122(void);
void int_start_123(void);
void int_start_124(void);
void int_start_125(void);
void int_start_126(void);
void int_start_127(void);
void int_start_128(void);
void int_start_129(void);
void int_start_130(void);
void int_start_131(void);
void int_start_132(void);
void int_start_133(void);
void int_start_134(void);
void int_start_135(void);
void int_start_136(void);
void int_start_137(void);
void int_start_138(void);
void int_start_139(void);
void int_start_140(void);
void int_start_141(void);
void int_start_142(void);
void int_start_143(void);
void int_start_144(void);
void int_start_145(void);
void int_start_146(void);
void int_start_147(void);
void int_start_148(void);
void int_start_149(void);
void int_start_150(void);
void int_start_151(void);
void int_start_152(void);
void int_start_153(void);
void int_start_154(void);
void int_start_155(void);
void int_start_156(void);
void int_start_157(void);
void int_start_158(void);
void int_start_159(void);
void int_start_160(void);
void int_start_161(void);
void int_start_162(void);
void int_start_163(void);
void int_start_164(void);
void int_start_165(void);
void int_start_166(void);
void int_start_167(void);
void int_start_168(void);
void int_start_169(void);
void int_start_170(void);
void int_start_171(void);
void int_start_172(void);
void int_start_173(void);
void int_start_174(void);
void int_start_175(void);
void int_start_176(void);
void int_start_177(void);
void int_start_178(void);
void int_start_179(void);
void int_start_180(void);
void int_start_181(void);
void int_start_182(void);
void int_start_183(void);
void int_start_184(void);
void int_start_185(void);
void int_start_186(void);
void int_start_187(void);
void int_start_188(void);
void int_start_189(void);
void int_start_190(void);
void int_start_191(void);
void int_start_192(void);
void int_start_193(void);
void int_start_194(void);
void int_start_195(void);
void int_start_196(void);
void int_start_197(void);
void int_start_198(void);
void int_start_199(void);
void int_start_200(void);
void int_start_201(void);
void int_start_202(void);
void int_start_203(void);
void int_start_204(void);
void int_start_205(void);
void int_start_206(void);
void int_start_207(void);
void int_start_208(void);
void int_start_209(void);
void int_start_210(void);
void int_start_211(void);
void int_start_212(void);
void int_start_213(void);
void int_start_214(void);
void int_start_215(void);
void int_start_216(void);
void int_start_217(void);
void int_start_218(void);
void int_start_219(void);
void int_start_220(void);
void int_start_221(void);
void int_start_222(void);
void int_start_223(void);
void int_start_224(void);
void int_start_225(void);
void int_start_226(void);
void int_start_227(void);
void int_start_228(void);
void int_start_229(void);
void int_start_230(void);
void int_start_231(void);
void int_start_232(void);
void int_start_233(void);
void int_start_234(void);
void int_start_235(void);
void int_start_236(void);
void int_start_237(void);
void int_start_238(void);
void int_start_239(void);
void int_start_240(void);
void int_start_241(void);
void int_start_242(void);
void int_start_243(void);
void int_start_244(void);
void int_start_245(void);
void int_start_246(void);
void int_start_247(void);
void int_start_248(void);
void int_start_249(void);
void int_start_250(void);
void int_start_251(void);
void int_start_252(void);
void int_start_253(void);
void int_start_254(void);
void int_start_255(void);



void interrupts_init() {
    pic_init();

    // Set up the IDT handlers.
    // This is suuuuper ugly, but I don't really see another way of doing this.
    uint16_t segment_selector = 0x8;
    uint8_t flags = IDT_GATE_INT32 | IDT_PRIV_3 | IDT_PRESENT;
    
    idt[0] = interrupts_create_descriptor(
        (uint32_t)exc_start_0,
        segment_selector,
        flags
    );

    idt[1] = interrupts_create_descriptor(
        (uint32_t)exc_start_1,
        segment_selector,
        flags
    );

    idt[2] = interrupts_create_descriptor(
        (uint32_t)exc_start_2,
        segment_selector,
        flags
    );

    idt[3] = interrupts_create_descriptor(
        (uint32_t)exc_start_3,
        segment_selector,
        flags
    );

    idt[4] = interrupts_create_descriptor(
        (uint32_t)exc_start_4,
        segment_selector,
        flags
    );

    idt[5] = interrupts_create_descriptor(
        (uint32_t)exc_start_5,
        segment_selector,
        flags
    );
    
    idt[6] = interrupts_create_descriptor(
        (uint32_t)exc_start_6,
        segment_selector,
        flags
    );

    idt[7] = interrupts_create_descriptor(
        (uint32_t)exc_start_7,
        segment_selector,
        flags
    );

    idt[8] = interrupts_create_descriptor(
        (uint32_t)exc_start_8,
        segment_selector,
        flags
    );
    
    idt[9] = interrupts_create_descriptor(
        (uint32_t)exc_start_9,
        segment_selector,
        flags
    );

    idt[10] = interrupts_create_descriptor(
        (uint32_t)exc_start_10,
        segment_selector,
        flags
    );

    idt[11] = interrupts_create_descriptor(
        (uint32_t)exc_start_11,
        segment_selector,
        flags
    );

    idt[12] = interrupts_create_descriptor(
        (uint32_t)exc_start_12,
        segment_selector,
        flags
    );

    idt[13] = interrupts_create_descriptor(
        (uint32_t)exc_start_13,
        segment_selector,
        flags
    );

    idt[14] = interrupts_create_descriptor(
        (uint32_t)exc_start_14,
        segment_selector,
        flags
    );

    idt[15] = interrupts_create_descriptor(
        (uint32_t)exc_start_15,
        segment_selector,
        flags
    );

    idt[16] = interrupts_create_descriptor(
        (uint32_t)exc_start_16,
        segment_selector,
        flags
    );

    idt[17] = interrupts_create_descriptor(
        (uint32_t)exc_start_17,
        segment_selector,
        flags
    );

    idt[18] = interrupts_create_descriptor(
        (uint32_t)exc_start_18,
        segment_selector,
        flags
    );

    idt[19] = interrupts_create_descriptor(
        (uint32_t)exc_start_19,
        segment_selector,
        flags
    );

    idt[20] = interrupts_create_descriptor(
        (uint32_t)exc_start_20,
        segment_selector,
        flags
    );

    idt[21] = interrupts_create_descriptor(
        (uint32_t)exc_start_21,
        segment_selector,
        flags
    );

    idt[22] = interrupts_create_descriptor(
        (uint32_t)exc_start_22,
        segment_selector,
        flags
    );

    idt[23] = interrupts_create_descriptor(
        (uint32_t)exc_start_23,
        segment_selector,
        flags
    );

    idt[24] = interrupts_create_descriptor(
        (uint32_t)exc_start_24,
        segment_selector,
        flags
    );

    idt[25] = interrupts_create_descriptor(
        (uint32_t)exc_start_25,
        segment_selector,
        flags
    );

    idt[26] = interrupts_create_descriptor(
        (uint32_t)exc_start_26,
        segment_selector,
        flags
    );

    idt[27] = interrupts_create_descriptor(
        (uint32_t)exc_start_27,
        segment_selector,
        flags
    );

    idt[28] = interrupts_create_descriptor(
        (uint32_t)exc_start_28,
        segment_selector,
        flags
    );

    idt[29] = interrupts_create_descriptor(
        (uint32_t)exc_start_29,
        segment_selector,
        flags
    );

    idt[30] = interrupts_create_descriptor(
        (uint32_t)exc_start_30,
        segment_selector,
        flags
    );

    idt[31] = interrupts_create_descriptor(
        (uint32_t)exc_start_31,
        segment_selector,
        flags
    );

    idt[32] = interrupts_create_descriptor(
        (uint32_t)int_start_32,
        segment_selector,
        flags
    );

    idt[33] = interrupts_create_descriptor(
        (uint32_t)int_start_33,
        segment_selector,
        flags
    );

    idt[34] = interrupts_create_descriptor(
        (uint32_t)int_start_34,
        segment_selector,
        flags
    );

    idt[35] = interrupts_create_descriptor(
        (uint32_t)int_start_35,
        segment_selector,
        flags
    );

    idt[36] = interrupts_create_descriptor(
        (uint32_t)int_start_36,
        segment_selector,
        flags
    );

    idt[37] = interrupts_create_descriptor(
        (uint32_t)int_start_37,
        segment_selector,
        flags
    );

    idt[38] = interrupts_create_descriptor(
        (uint32_t)int_start_38,
        segment_selector,
        flags
    );

    idt[39] = interrupts_create_descriptor(
        (uint32_t)int_start_39,
        segment_selector,
        flags
    );

    idt[40] = interrupts_create_descriptor(
        (uint32_t)int_start_40,
        segment_selector,
        flags
    );

    idt[41] = interrupts_create_descriptor(
        (uint32_t)int_start_41,
        segment_selector,
        flags
    );

    idt[42] = interrupts_create_descriptor(
        (uint32_t)int_start_42,
        segment_selector,
        flags
    );

    idt[43] = interrupts_create_descriptor(
        (uint32_t)int_start_43,
        segment_selector,
        flags
    );

    idt[44] = interrupts_create_descriptor(
        (uint32_t)int_start_44,
        segment_selector,
        flags
    );

    idt[45] = interrupts_create_descriptor(
        (uint32_t)int_start_45,
        segment_selector,
        flags
    );

    idt[46] = interrupts_create_descriptor(
        (uint32_t)int_start_46,
        segment_selector,
        flags
    );

    idt[47] = interrupts_create_descriptor(
        (uint32_t)int_start_47,
        segment_selector,
        flags
    );

    idt[48] = interrupts_create_descriptor(
        (uint32_t)int_start_48,
        segment_selector,
        flags
    );

    idt[49] = interrupts_create_descriptor(
        (uint32_t)int_start_49,
        segment_selector,
        flags
    );

    idt[50] = interrupts_create_descriptor(
        (uint32_t)int_start_50,
        segment_selector,
        flags
    );

    idt[51] = interrupts_create_descriptor(
        (uint32_t)int_start_51,
        segment_selector,
        flags
    );

    idt[52] = interrupts_create_descriptor(
        (uint32_t)int_start_52,
        segment_selector,
        flags
    );

    idt[53] = interrupts_create_descriptor(
        (uint32_t)int_start_53,
        segment_selector,
        flags
    );

    idt[54] = interrupts_create_descriptor(
        (uint32_t)int_start_54,
        segment_selector,
        flags
    );

    idt[55] = interrupts_create_descriptor(
        (uint32_t)int_start_55,
        segment_selector,
        flags
    );

    idt[56] = interrupts_create_descriptor(
        (uint32_t)int_start_56,
        segment_selector,
        flags
    );

    idt[57] = interrupts_create_descriptor(
        (uint32_t)int_start_57,
        segment_selector,
        flags
    );

    idt[58] = interrupts_create_descriptor(
        (uint32_t)int_start_58,
        segment_selector,
        flags
    );

    idt[59] = interrupts_create_descriptor(
        (uint32_t)int_start_59,
        segment_selector,
        flags
    );

    idt[60] = interrupts_create_descriptor(
        (uint32_t)int_start_60,
        segment_selector,
        flags
    );

    idt[61] = interrupts_create_descriptor(
        (uint32_t)int_start_61,
        segment_selector,
        flags
    );

    idt[62] = interrupts_create_descriptor(
        (uint32_t)int_start_62,
        segment_selector,
        flags
    );

    idt[63] = interrupts_create_descriptor(
        (uint32_t)int_start_63,
        segment_selector,
        flags
    );

    idt[64] = interrupts_create_descriptor(
        (uint32_t)int_start_64,
        segment_selector,
        flags
    );

    idt[65] = interrupts_create_descriptor(
        (uint32_t)int_start_65,
        segment_selector,
        flags
    );

    idt[66] = interrupts_create_descriptor(
        (uint32_t)int_start_66,
        segment_selector,
        flags
    );

    idt[67] = interrupts_create_descriptor(
        (uint32_t)int_start_67,
        segment_selector,
        flags
    );

    idt[68] = interrupts_create_descriptor(
        (uint32_t)int_start_68,
        segment_selector,
        flags
    );

    idt[69] = interrupts_create_descriptor(
        (uint32_t)int_start_69,
        segment_selector,
        flags
    );

    idt[70] = interrupts_create_descriptor(
        (uint32_t)int_start_70,
        segment_selector,
        flags
    );

    idt[71] = interrupts_create_descriptor(
        (uint32_t)int_start_71,
        segment_selector,
        flags
    );

    idt[72] = interrupts_create_descriptor(
        (uint32_t)int_start_72,
        segment_selector,
        flags
    );

    idt[73] = interrupts_create_descriptor(
        (uint32_t)int_start_73,
        segment_selector,
        flags
    );

    idt[74] = interrupts_create_descriptor(
        (uint32_t)int_start_74,
        segment_selector,
        flags
    );

    idt[75] = interrupts_create_descriptor(
        (uint32_t)int_start_75,
        segment_selector,
        flags
    );

    idt[76] = interrupts_create_descriptor(
        (uint32_t)int_start_76,
        segment_selector,
        flags
    );

    idt[77] = interrupts_create_descriptor(
        (uint32_t)int_start_77,
        segment_selector,
        flags
    );

    idt[78] = interrupts_create_descriptor(
        (uint32_t)int_start_78,
        segment_selector,
        flags
    );

    idt[79] = interrupts_create_descriptor(
        (uint32_t)int_start_79,
        segment_selector,
        flags
    );

    idt[80] = interrupts_create_descriptor(
        (uint32_t)int_start_80,
        segment_selector,
        flags
    );

    idt[81] = interrupts_create_descriptor(
        (uint32_t)int_start_81,
        segment_selector,
        flags
    );

    idt[82] = interrupts_create_descriptor(
        (uint32_t)int_start_82,
        segment_selector,
        flags
    );

    idt[83] = interrupts_create_descriptor(
        (uint32_t)int_start_83,
        segment_selector,
        flags
    );

    idt[84] = interrupts_create_descriptor(
        (uint32_t)int_start_84,
        segment_selector,
        flags
    );

    idt[85] = interrupts_create_descriptor(
        (uint32_t)int_start_85,
        segment_selector,
        flags
    );

    idt[86] = interrupts_create_descriptor(
        (uint32_t)int_start_86,
        segment_selector,
        flags
    );

    idt[87] = interrupts_create_descriptor(
        (uint32_t)int_start_87,
        segment_selector,
        flags
    );

    idt[88] = interrupts_create_descriptor(
        (uint32_t)int_start_88,
        segment_selector,
        flags
    );

    idt[89] = interrupts_create_descriptor(
        (uint32_t)int_start_89,
        segment_selector,
        flags
    );

    idt[90] = interrupts_create_descriptor(
        (uint32_t)int_start_90,
        segment_selector,
        flags
    );

    idt[91] = interrupts_create_descriptor(
        (uint32_t)int_start_91,
        segment_selector,
        flags
    );

    idt[92] = interrupts_create_descriptor(
        (uint32_t)int_start_92,
        segment_selector,
        flags
    );

    idt[93] = interrupts_create_descriptor(
        (uint32_t)int_start_93,
        segment_selector,
        flags
    );

    idt[94] = interrupts_create_descriptor(
        (uint32_t)int_start_94,
        segment_selector,
        flags
    );

    idt[95] = interrupts_create_descriptor(
        (uint32_t)int_start_95,
        segment_selector,
        flags
    );

    idt[96] = interrupts_create_descriptor(
        (uint32_t)int_start_96,
        segment_selector,
        flags
    );

    idt[97] = interrupts_create_descriptor(
        (uint32_t)int_start_97,
        segment_selector,
        flags
    );

    idt[98] = interrupts_create_descriptor(
        (uint32_t)int_start_98,
        segment_selector,
        flags
    );

    idt[99] = interrupts_create_descriptor(
        (uint32_t)int_start_99,
        segment_selector,
        flags
    );

    idt[100] = interrupts_create_descriptor(
        (uint32_t)int_start_100,
        segment_selector,
        flags
    );

    idt[101] = interrupts_create_descriptor(
        (uint32_t)int_start_101,
        segment_selector,
        flags
    );

    idt[102] = interrupts_create_descriptor(
        (uint32_t)int_start_102,
        segment_selector,
        flags
    );

    idt[103] = interrupts_create_descriptor(
        (uint32_t)int_start_103,
        segment_selector,
        flags
    );

    idt[104] = interrupts_create_descriptor(
        (uint32_t)int_start_104,
        segment_selector,
        flags
    );

    idt[105] = interrupts_create_descriptor(
        (uint32_t)int_start_105,
        segment_selector,
        flags
    );

    idt[106] = interrupts_create_descriptor(
        (uint32_t)int_start_106,
        segment_selector,
        flags
    );

    idt[107] = interrupts_create_descriptor(
        (uint32_t)int_start_107,
        segment_selector,
        flags
    );

    idt[108] = interrupts_create_descriptor(
        (uint32_t)int_start_108,
        segment_selector,
        flags
    );

    idt[109] = interrupts_create_descriptor(
        (uint32_t)int_start_109,
        segment_selector,
        flags
    );

    idt[110] = interrupts_create_descriptor(
        (uint32_t)int_start_110,
        segment_selector,
        flags
    );

    idt[111] = interrupts_create_descriptor(
        (uint32_t)int_start_111,
        segment_selector,
        flags
    );

    idt[112] = interrupts_create_descriptor(
        (uint32_t)int_start_112,
        segment_selector,
        flags
    );

    idt[113] = interrupts_create_descriptor(
        (uint32_t)int_start_113,
        segment_selector,
        flags
    );

    idt[114] = interrupts_create_descriptor(
        (uint32_t)int_start_114,
        segment_selector,
        flags
    );

    idt[115] = interrupts_create_descriptor(
        (uint32_t)int_start_115,
        segment_selector,
        flags
    );

    idt[116] = interrupts_create_descriptor(
        (uint32_t)int_start_116,
        segment_selector,
        flags
    );

    idt[117] = interrupts_create_descriptor(
        (uint32_t)int_start_117,
        segment_selector,
        flags
    );

    idt[118] = interrupts_create_descriptor(
        (uint32_t)int_start_118,
        segment_selector,
        flags
    );

    idt[119] = interrupts_create_descriptor(
        (uint32_t)int_start_119,
        segment_selector,
        flags
    );

    idt[120] = interrupts_create_descriptor(
        (uint32_t)int_start_120,
        segment_selector,
        flags
    );

    idt[121] = interrupts_create_descriptor(
        (uint32_t)int_start_121,
        segment_selector,
        flags
    );

    idt[122] = interrupts_create_descriptor(
        (uint32_t)int_start_122,
        segment_selector,
        flags
    );

    idt[123] = interrupts_create_descriptor(
        (uint32_t)int_start_123,
        segment_selector,
        flags
    );

    idt[124] = interrupts_create_descriptor(
        (uint32_t)int_start_124,
        segment_selector,
        flags
    );

    idt[125] = interrupts_create_descriptor(
        (uint32_t)int_start_125,
        segment_selector,
        flags
    );

    idt[126] = interrupts_create_descriptor(
        (uint32_t)int_start_126,
        segment_selector,
        flags
    );

    idt[127] = interrupts_create_descriptor(
        (uint32_t)int_start_127,
        segment_selector,
        flags
    );

    idt[128] = interrupts_create_descriptor(
        (uint32_t)int_start_128,
        segment_selector,
        flags
    );

    idt[129] = interrupts_create_descriptor(
        (uint32_t)int_start_129,
        segment_selector,
        flags
    );

    idt[130] = interrupts_create_descriptor(
        (uint32_t)int_start_130,
        segment_selector,
        flags
    );

    idt[131] = interrupts_create_descriptor(
        (uint32_t)int_start_131,
        segment_selector,
        flags
    );

    idt[132] = interrupts_create_descriptor(
        (uint32_t)int_start_132,
        segment_selector,
        flags
    );

    idt[133] = interrupts_create_descriptor(
        (uint32_t)int_start_133,
        segment_selector,
        flags
    );

    idt[134] = interrupts_create_descriptor(
        (uint32_t)int_start_134,
        segment_selector,
        flags
    );

    idt[135] = interrupts_create_descriptor(
        (uint32_t)int_start_135,
        segment_selector,
        flags
    );

    idt[136] = interrupts_create_descriptor(
        (uint32_t)int_start_136,
        segment_selector,
        flags
    );

    idt[137] = interrupts_create_descriptor(
        (uint32_t)int_start_137,
        segment_selector,
        flags
    );

    idt[138] = interrupts_create_descriptor(
        (uint32_t)int_start_138,
        segment_selector,
        flags
    );

    idt[139] = interrupts_create_descriptor(
        (uint32_t)int_start_139,
        segment_selector,
        flags
    );

    idt[140] = interrupts_create_descriptor(
        (uint32_t)int_start_140,
        segment_selector,
        flags
    );

    idt[141] = interrupts_create_descriptor(
        (uint32_t)int_start_141,
        segment_selector,
        flags
    );

    idt[142] = interrupts_create_descriptor(
        (uint32_t)int_start_142,
        segment_selector,
        flags
    );

    idt[143] = interrupts_create_descriptor(
        (uint32_t)int_start_143,
        segment_selector,
        flags
    );

    idt[144] = interrupts_create_descriptor(
        (uint32_t)int_start_144,
        segment_selector,
        flags
    );

    idt[145] = interrupts_create_descriptor(
        (uint32_t)int_start_145,
        segment_selector,
        flags
    );

    idt[146] = interrupts_create_descriptor(
        (uint32_t)int_start_146,
        segment_selector,
        flags
    );

    idt[147] = interrupts_create_descriptor(
        (uint32_t)int_start_147,
        segment_selector,
        flags
    );

    idt[148] = interrupts_create_descriptor(
        (uint32_t)int_start_148,
        segment_selector,
        flags
    );

    idt[149] = interrupts_create_descriptor(
        (uint32_t)int_start_149,
        segment_selector,
        flags
    );

    idt[150] = interrupts_create_descriptor(
        (uint32_t)int_start_150,
        segment_selector,
        flags
    );

    idt[151] = interrupts_create_descriptor(
        (uint32_t)int_start_151,
        segment_selector,
        flags
    );

    idt[152] = interrupts_create_descriptor(
        (uint32_t)int_start_152,
        segment_selector,
        flags
    );

    idt[153] = interrupts_create_descriptor(
        (uint32_t)int_start_153,
        segment_selector,
        flags
    );

    idt[154] = interrupts_create_descriptor(
        (uint32_t)int_start_154,
        segment_selector,
        flags
    );

    idt[155] = interrupts_create_descriptor(
        (uint32_t)int_start_155,
        segment_selector,
        flags
    );

    idt[156] = interrupts_create_descriptor(
        (uint32_t)int_start_156,
        segment_selector,
        flags
    );

    idt[157] = interrupts_create_descriptor(
        (uint32_t)int_start_157,
        segment_selector,
        flags
    );

    idt[158] = interrupts_create_descriptor(
        (uint32_t)int_start_158,
        segment_selector,
        flags
    );

    idt[159] = interrupts_create_descriptor(
        (uint32_t)int_start_159,
        segment_selector,
        flags
    );

    idt[160] = interrupts_create_descriptor(
        (uint32_t)int_start_160,
        segment_selector,
        flags
    );

    idt[161] = interrupts_create_descriptor(
        (uint32_t)int_start_161,
        segment_selector,
        flags
    );

    idt[162] = interrupts_create_descriptor(
        (uint32_t)int_start_162,
        segment_selector,
        flags
    );

    idt[163] = interrupts_create_descriptor(
        (uint32_t)int_start_163,
        segment_selector,
        flags
    );

    idt[164] = interrupts_create_descriptor(
        (uint32_t)int_start_164,
        segment_selector,
        flags
    );

    idt[165] = interrupts_create_descriptor(
        (uint32_t)int_start_165,
        segment_selector,
        flags
    );

    idt[166] = interrupts_create_descriptor(
        (uint32_t)int_start_166,
        segment_selector,
        flags
    );

    idt[167] = interrupts_create_descriptor(
        (uint32_t)int_start_167,
        segment_selector,
        flags
    );

    idt[168] = interrupts_create_descriptor(
        (uint32_t)int_start_168,
        segment_selector,
        flags
    );

    idt[169] = interrupts_create_descriptor(
        (uint32_t)int_start_169,
        segment_selector,
        flags
    );

    idt[170] = interrupts_create_descriptor(
        (uint32_t)int_start_170,
        segment_selector,
        flags
    );

    idt[171] = interrupts_create_descriptor(
        (uint32_t)int_start_171,
        segment_selector,
        flags
    );

    idt[172] = interrupts_create_descriptor(
        (uint32_t)int_start_172,
        segment_selector,
        flags
    );

    idt[173] = interrupts_create_descriptor(
        (uint32_t)int_start_173,
        segment_selector,
        flags
    );

    idt[174] = interrupts_create_descriptor(
        (uint32_t)int_start_174,
        segment_selector,
        flags
    );

    idt[175] = interrupts_create_descriptor(
        (uint32_t)int_start_175,
        segment_selector,
        flags
    );

    idt[176] = interrupts_create_descriptor(
        (uint32_t)int_start_176,
        segment_selector,
        flags
    );

    idt[177] = interrupts_create_descriptor(
        (uint32_t)int_start_177,
        segment_selector,
        flags
    );

    idt[178] = interrupts_create_descriptor(
        (uint32_t)int_start_178,
        segment_selector,
        flags
    );

    idt[179] = interrupts_create_descriptor(
        (uint32_t)int_start_179,
        segment_selector,
        flags
    );

    idt[180] = interrupts_create_descriptor(
        (uint32_t)int_start_180,
        segment_selector,
        flags
    );

    idt[181] = interrupts_create_descriptor(
        (uint32_t)int_start_181,
        segment_selector,
        flags
    );

    idt[182] = interrupts_create_descriptor(
        (uint32_t)int_start_182,
        segment_selector,
        flags
    );

    idt[183] = interrupts_create_descriptor(
        (uint32_t)int_start_183,
        segment_selector,
        flags
    );

    idt[184] = interrupts_create_descriptor(
        (uint32_t)int_start_184,
        segment_selector,
        flags
    );

    idt[185] = interrupts_create_descriptor(
        (uint32_t)int_start_185,
        segment_selector,
        flags
    );

    idt[186] = interrupts_create_descriptor(
        (uint32_t)int_start_186,
        segment_selector,
        flags
    );

    idt[187] = interrupts_create_descriptor(
        (uint32_t)int_start_187,
        segment_selector,
        flags
    );

    idt[188] = interrupts_create_descriptor(
        (uint32_t)int_start_188,
        segment_selector,
        flags
    );

    idt[189] = interrupts_create_descriptor(
        (uint32_t)int_start_189,
        segment_selector,
        flags
    );

    idt[190] = interrupts_create_descriptor(
        (uint32_t)int_start_190,
        segment_selector,
        flags
    );

    idt[191] = interrupts_create_descriptor(
        (uint32_t)int_start_191,
        segment_selector,
        flags
    );

    idt[192] = interrupts_create_descriptor(
        (uint32_t)int_start_192,
        segment_selector,
        flags
    );

    idt[193] = interrupts_create_descriptor(
        (uint32_t)int_start_193,
        segment_selector,
        flags
    );

    idt[194] = interrupts_create_descriptor(
        (uint32_t)int_start_194,
        segment_selector,
        flags
    );

    idt[195] = interrupts_create_descriptor(
        (uint32_t)int_start_195,
        segment_selector,
        flags
    );

    idt[196] = interrupts_create_descriptor(
        (uint32_t)int_start_196,
        segment_selector,
        flags
    );

    idt[197] = interrupts_create_descriptor(
        (uint32_t)int_start_197,
        segment_selector,
        flags
    );

    idt[198] = interrupts_create_descriptor(
        (uint32_t)int_start_198,
        segment_selector,
        flags
    );

    idt[199] = interrupts_create_descriptor(
        (uint32_t)int_start_199,
        segment_selector,
        flags
    );

    idt[200] = interrupts_create_descriptor(
        (uint32_t)int_start_200,
        segment_selector,
        flags
    );

    idt[201] = interrupts_create_descriptor(
        (uint32_t)int_start_201,
        segment_selector,
        flags
    );

    idt[202] = interrupts_create_descriptor(
        (uint32_t)int_start_202,
        segment_selector,
        flags
    );

    idt[203] = interrupts_create_descriptor(
        (uint32_t)int_start_203,
        segment_selector,
        flags
    );

    idt[204] = interrupts_create_descriptor(
        (uint32_t)int_start_204,
        segment_selector,
        flags
    );

    idt[205] = interrupts_create_descriptor(
        (uint32_t)int_start_205,
        segment_selector,
        flags
    );

    idt[206] = interrupts_create_descriptor(
        (uint32_t)int_start_206,
        segment_selector,
        flags
    );

    idt[207] = interrupts_create_descriptor(
        (uint32_t)int_start_207,
        segment_selector,
        flags
    );

    idt[208] = interrupts_create_descriptor(
        (uint32_t)int_start_208,
        segment_selector,
        flags
    );

    idt[209] = interrupts_create_descriptor(
        (uint32_t)int_start_209,
        segment_selector,
        flags
    );

    idt[210] = interrupts_create_descriptor(
        (uint32_t)int_start_210,
        segment_selector,
        flags
    );

    idt[211] = interrupts_create_descriptor(
        (uint32_t)int_start_211,
        segment_selector,
        flags
    );

    idt[212] = interrupts_create_descriptor(
        (uint32_t)int_start_212,
        segment_selector,
        flags
    );

    idt[213] = interrupts_create_descriptor(
        (uint32_t)int_start_213,
        segment_selector,
        flags
    );

    idt[214] = interrupts_create_descriptor(
        (uint32_t)int_start_214,
        segment_selector,
        flags
    );

    idt[215] = interrupts_create_descriptor(
        (uint32_t)int_start_215,
        segment_selector,
        flags
    );

    idt[216] = interrupts_create_descriptor(
        (uint32_t)int_start_216,
        segment_selector,
        flags
    );

    idt[217] = interrupts_create_descriptor(
        (uint32_t)int_start_217,
        segment_selector,
        flags
    );

    idt[218] = interrupts_create_descriptor(
        (uint32_t)int_start_218,
        segment_selector,
        flags
    );

    idt[219] = interrupts_create_descriptor(
        (uint32_t)int_start_219,
        segment_selector,
        flags
    );

    idt[220] = interrupts_create_descriptor(
        (uint32_t)int_start_220,
        segment_selector,
        flags
    );

    idt[221] = interrupts_create_descriptor(
        (uint32_t)int_start_221,
        segment_selector,
        flags
    );

    idt[222] = interrupts_create_descriptor(
        (uint32_t)int_start_222,
        segment_selector,
        flags
    );

    idt[223] = interrupts_create_descriptor(
        (uint32_t)int_start_223,
        segment_selector,
        flags
    );

    idt[224] = interrupts_create_descriptor(
        (uint32_t)int_start_224,
        segment_selector,
        flags
    );

    idt[225] = interrupts_create_descriptor(
        (uint32_t)int_start_225,
        segment_selector,
        flags
    );

    idt[226] = interrupts_create_descriptor(
        (uint32_t)int_start_226,
        segment_selector,
        flags
    );

    idt[227] = interrupts_create_descriptor(
        (uint32_t)int_start_227,
        segment_selector,
        flags
    );

    idt[228] = interrupts_create_descriptor(
        (uint32_t)int_start_228,
        segment_selector,
        flags
    );

    idt[229] = interrupts_create_descriptor(
        (uint32_t)int_start_229,
        segment_selector,
        flags
    );

    idt[230] = interrupts_create_descriptor(
        (uint32_t)int_start_230,
        segment_selector,
        flags
    );

    idt[231] = interrupts_create_descriptor(
        (uint32_t)int_start_231,
        segment_selector,
        flags
    );

    idt[232] = interrupts_create_descriptor(
        (uint32_t)int_start_232,
        segment_selector,
        flags
    );

    idt[233] = interrupts_create_descriptor(
        (uint32_t)int_start_233,
        segment_selector,
        flags
    );

    idt[234] = interrupts_create_descriptor(
        (uint32_t)int_start_234,
        segment_selector,
        flags
    );

    idt[235] = interrupts_create_descriptor(
        (uint32_t)int_start_235,
        segment_selector,
        flags
    );

    idt[236] = interrupts_create_descriptor(
        (uint32_t)int_start_236,
        segment_selector,
        flags
    );

    idt[237] = interrupts_create_descriptor(
        (uint32_t)int_start_237,
        segment_selector,
        flags
    );

    idt[238] = interrupts_create_descriptor(
        (uint32_t)int_start_238,
        segment_selector,
        flags
    );

    idt[239] = interrupts_create_descriptor(
        (uint32_t)int_start_239,
        segment_selector,
        flags
    );

    idt[240] = interrupts_create_descriptor(
        (uint32_t)int_start_240,
        segment_selector,
        flags
    );

    idt[241] = interrupts_create_descriptor(
        (uint32_t)int_start_241,
        segment_selector,
        flags
    );

    idt[242] = interrupts_create_descriptor(
        (uint32_t)int_start_242,
        segment_selector,
        flags
    );

    idt[243] = interrupts_create_descriptor(
        (uint32_t)int_start_243,
        segment_selector,
        flags
    );

    idt[244] = interrupts_create_descriptor(
        (uint32_t)int_start_244,
        segment_selector,
        flags
    );

    idt[245] = interrupts_create_descriptor(
        (uint32_t)int_start_245,
        segment_selector,
        flags
    );

    idt[246] = interrupts_create_descriptor(
        (uint32_t)int_start_246,
        segment_selector,
        flags
    );

    idt[247] = interrupts_create_descriptor(
        (uint32_t)int_start_247,
        segment_selector,
        flags
    );

    idt[248] = interrupts_create_descriptor(
        (uint32_t)int_start_248,
        segment_selector,
        flags
    );

    idt[249] = interrupts_create_descriptor(
        (uint32_t)int_start_249,
        segment_selector,
        flags
    );

    idt[250] = interrupts_create_descriptor(
        (uint32_t)int_start_250,
        segment_selector,
        flags
    );

    idt[251] = interrupts_create_descriptor(
        (uint32_t)int_start_251,
        segment_selector,
        flags
    );

    idt[252] = interrupts_create_descriptor(
        (uint32_t)int_start_252,
        segment_selector,
        flags
    );

    idt[253] = interrupts_create_descriptor(
        (uint32_t)int_start_253,
        segment_selector,
        flags
    );

    idt[254] = interrupts_create_descriptor(
        (uint32_t)int_start_254,
        segment_selector,
        flags
    );

    idt[255] = interrupts_create_descriptor(
        (uint32_t)int_start_255,
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

void c_exception_handler(int vector, Interrupt_Frame *frame) {
    fmt_print("Got exception %d!\n", vector);
    fmt_print("  edi: %x\n", frame->edi);
    fmt_print("  esi: %x\n", frame->esi);
    fmt_print("  ebp: %x\n", frame->ebp);
    fmt_print("  ebx: %x\n", frame->ebx);
    fmt_print("  edx: %x\n", frame->edx);
    fmt_print("  ecx: %x\n", frame->ecx);
    fmt_print("  eax: %x\n", frame->eax);
    fmt_print("  error_code: %hx\n", frame->error_code);
    fmt_print("  eip: %x\n", frame->eip);
    fmt_print("  cs: %hx\n", frame->cs);
    fmt_print("  eflags: %x\n", frame->eflags);
    fmt_print("  esp: %x\n", frame->esp);
    fmt_print("  ss: %hx\n", frame->ss);
    cpu_halt();
}

void c_interrupt_dispatch(int vector, Interrupt_Frame *frame) {
    registered_handlers[vector - 32](frame);
}

void interrupts_register_handler(int vector, Registered_Handler handler) {
    // @FIXME: This should probably be an atomic load.
    registered_handlers[vector - 32] = handler;
}
