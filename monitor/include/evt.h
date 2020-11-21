# Vector definitions
###########################################################################
.ifndef EXCEPT_RESET_SSP
	EXCEPT_RESET_SSP = 0x00000000
.endif
.ifndef EXCEPT_RESET_PC
	EXCEPT_RESET_PC = 0x00000500
.endif
.ifndef EXCEPT_ACCESS_ERROR
	EXCEPT_ACCESS_ERROR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_ADDRESS_ERROR
	EXCEPT_ADDRESS_ERROR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_ILLEGAL_INSTRUCTION
	EXCEPT_ILLEGAL_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_DIVIDE_BY_ZERO
	EXCEPT_DIVIDE_BY_ZERO = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_CHK_INSTRUCTION
	EXCEPT_CHK_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAPV_INSTRUCTION
	EXCEPT_TRAPV_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_PRIVILEGE_VIOLATION
	EXCEPT_PRIVILEGE_VIOLATION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRACE
	EXCEPT_TRACE = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_LINE_A_EMULATOR
	EXCEPT_LINE_A_EMULATOR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_LINE_F_EMULATOR
	EXCEPT_LINE_F_EMULATOR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_RESERVED
	EXCEPT_RESERVED = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_COPROCESSOR_VIOLATION
	EXCEPT_COPROCESSOR_VIOLATION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FORMAT_ERROR
	EXCEPT_FORMAT_ERROR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_UNINITIALEZED_INTERRUPT
	EXCEPT_UNINITIALEZED_INTERRUPT = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_SUPRIOUS_INTERRUPT
	EXCEPT_SUPRIOUS_INTERRUPT = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL1
	EXCEPT_INTERRUPT_LEVEL1 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL2
	EXCEPT_INTERRUPT_LEVEL2	= EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL3
	EXCEPT_INTERRUPT_LEVEL3 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL4
	EXCEPT_INTERRUPT_LEVEL4 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL5
	EXCEPT_INTERRUPT_LEVEL5 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL6
	EXCEPT_INTERRUPT_LEVEL6 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_INTERRUPT_LEVEL7
	EXCEPT_INTERRUPT_LEVEL7 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP0_INSTRUCTION
	EXCEPT_TRAP0_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP1_INSTRUCTION
	EXCEPT_TRAP1_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP2_INSTRUCTION
	EXCEPT_TRAP2_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP3_INSTRUCTION
	EXCEPT_TRAP3_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP4_INSTRUCTION
	EXCEPT_TRAP4_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP5_INSTRUCTION
	EXCEPT_TRAP5_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP6_INSTRUCTION
	EXCEPT_TRAP6_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP7_INSTRUCTION
	EXCEPT_TRAP7_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP8_INSTRUCTION
	EXCEPT_TRAP8_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP9_INSTRUCTION
	EXCEPT_TRAP9_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP10_INSTRUCTION
	EXCEPT_TRAP10_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP11_INSTRUCTION
	EXCEPT_TRAP11_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP12_INSTRUCTION
	EXCEPT_TRAP12_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP13_INSTRUCTION
	EXCEPT_TRAP13_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP14_INSTRUCTION
	EXCEPT_TRAP14_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_TRAP15_INSTRUCTION
	EXCEPT_TRAP15_INSTRUCTION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_BRANCH_SET
	EXCEPT_FP_BRANCH_SET = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_INEXACT_RESULT
	EXCEPT_FP_INEXACT_RESULT = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_DIVIDE_BY_ZERO
	EXCEPT_FP_DIVIDE_BY_ZERO = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_UNDERFLOW
	EXCEPT_FP_UNDERFLOW = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_OPERAND_ERROR
	EXCEPT_FP_OPERAND_ERROR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_OVERFLOW
	EXCEPT_FP_OVERFLOW = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_SIGNALING_NAN
	EXCEPT_FP_SIGNALING_NAN = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_FP_UNIMPLEMENTED_DATA_TYPE
	EXCEPT_FP_UNIMPLEMENTED_DATA_TYPE = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_MMU_CONFIGURATION_ERROR
	EXCEPT_MMU_CONFIGURATION_ERROR = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_MMU_ILLEGAL_OPERATION
	EXCEPT_MMU_ILLEGAL_OPERATION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_MMU_ACCESS_LEVEL_VIOLATION
	EXCEPT_MMU_ACCESS_LEVEL_VIOLATION = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_0
	EXCEPT_USER_DEFINED_0 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_1
	EXCEPT_USER_DEFINED_1 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_2
	EXCEPT_USER_DEFINED_2 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_3
	EXCEPT_USER_DEFINED_3 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_4
	EXCEPT_USER_DEFINED_4 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_5
	EXCEPT_USER_DEFINED_5 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_6
	EXCEPT_USER_DEFINED_6 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_7
	EXCEPT_USER_DEFINED_7 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_8
	EXCEPT_USER_DEFINED_8 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_9
	EXCEPT_USER_DEFINED_9 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_10
	EXCEPT_USER_DEFINED_10 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_11
	EXCEPT_USER_DEFINED_11 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_12
	EXCEPT_USER_DEFINED_12 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_13
	EXCEPT_USER_DEFINED_13 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_14
	EXCEPT_USER_DEFINED_14 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_15
	EXCEPT_USER_DEFINED_15 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_16
	EXCEPT_USER_DEFINED_16 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_17
	EXCEPT_USER_DEFINED_17 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_18
	EXCEPT_USER_DEFINED_18 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_19
	EXCEPT_USER_DEFINED_19 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_20
	EXCEPT_USER_DEFINED_20 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_21
	EXCEPT_USER_DEFINED_21 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_22
	EXCEPT_USER_DEFINED_22 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_23
	EXCEPT_USER_DEFINED_23 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_24
	EXCEPT_USER_DEFINED_24 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_25
	EXCEPT_USER_DEFINED_25 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_26
	EXCEPT_USER_DEFINED_26 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_27
	EXCEPT_USER_DEFINED_27 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_28
	EXCEPT_USER_DEFINED_28 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_29
	EXCEPT_USER_DEFINED_29 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_30
	EXCEPT_USER_DEFINED_30 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_31
	EXCEPT_USER_DEFINED_31 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_32
	EXCEPT_USER_DEFINED_32 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_33
	EXCEPT_USER_DEFINED_33 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_34
	EXCEPT_USER_DEFINED_34 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_35
	EXCEPT_USER_DEFINED_35 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_36
	EXCEPT_USER_DEFINED_36 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_37
	EXCEPT_USER_DEFINED_37 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_38
	EXCEPT_USER_DEFINED_38 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_39
	EXCEPT_USER_DEFINED_39 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_40
	EXCEPT_USER_DEFINED_40 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_41
	EXCEPT_USER_DEFINED_41 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_42
	EXCEPT_USER_DEFINED_42 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_43
	EXCEPT_USER_DEFINED_43 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_44
	EXCEPT_USER_DEFINED_44 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_45
	EXCEPT_USER_DEFINED_45 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_46
	EXCEPT_USER_DEFINED_46 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_47
	EXCEPT_USER_DEFINED_47 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_48
	EXCEPT_USER_DEFINED_48 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_49
	EXCEPT_USER_DEFINED_49 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_50
	EXCEPT_USER_DEFINED_50 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_51
	EXCEPT_USER_DEFINED_51 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_52
	EXCEPT_USER_DEFINED_52 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_53
	EXCEPT_USER_DEFINED_53 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_54
	EXCEPT_USER_DEFINED_54 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_55
	EXCEPT_USER_DEFINED_55 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_56
	EXCEPT_USER_DEFINED_56 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_57
	EXCEPT_USER_DEFINED_57 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_58
	EXCEPT_USER_DEFINED_58 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_59
	EXCEPT_USER_DEFINED_59 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_60
	EXCEPT_USER_DEFINED_60 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_61
	EXCEPT_USER_DEFINED_61 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_62
	EXCEPT_USER_DEFINED_62 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_63
	EXCEPT_USER_DEFINED_63 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_64
	EXCEPT_USER_DEFINED_64 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_65
	EXCEPT_USER_DEFINED_65 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_66
	EXCEPT_USER_DEFINED_66 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_67
	EXCEPT_USER_DEFINED_67 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_68
	EXCEPT_USER_DEFINED_68 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_69
	EXCEPT_USER_DEFINED_69 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_70
	EXCEPT_USER_DEFINED_70 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_71
	EXCEPT_USER_DEFINED_71 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_72
	EXCEPT_USER_DEFINED_72 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_73
	EXCEPT_USER_DEFINED_73 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_74
	EXCEPT_USER_DEFINED_74 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_75
	EXCEPT_USER_DEFINED_75 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_76
	EXCEPT_USER_DEFINED_76 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_77
	EXCEPT_USER_DEFINED_77 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_78
	EXCEPT_USER_DEFINED_78 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_79
	EXCEPT_USER_DEFINED_79 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_80
	EXCEPT_USER_DEFINED_80 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_81
	EXCEPT_USER_DEFINED_81 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_82
	EXCEPT_USER_DEFINED_82 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_83
	EXCEPT_USER_DEFINED_83 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_84
	EXCEPT_USER_DEFINED_84 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_85
	EXCEPT_USER_DEFINED_85 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_86
	EXCEPT_USER_DEFINED_86 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_87
	EXCEPT_USER_DEFINED_87 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_88
	EXCEPT_USER_DEFINED_88 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_89
	EXCEPT_USER_DEFINED_89 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_90
	EXCEPT_USER_DEFINED_90 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_91
	EXCEPT_USER_DEFINED_91 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_92
	EXCEPT_USER_DEFINED_92 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_93
	EXCEPT_USER_DEFINED_93 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_94
	EXCEPT_USER_DEFINED_94 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_95
	EXCEPT_USER_DEFINED_95 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_96
	EXCEPT_USER_DEFINED_96 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_97
	EXCEPT_USER_DEFINED_97 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_98
	EXCEPT_USER_DEFINED_98 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_99
	EXCEPT_USER_DEFINED_99 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_100
	EXCEPT_USER_DEFINED_100 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_101
	EXCEPT_USER_DEFINED_101 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_102
	EXCEPT_USER_DEFINED_102 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_103
	EXCEPT_USER_DEFINED_103 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_104
	EXCEPT_USER_DEFINED_104 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_105
	EXCEPT_USER_DEFINED_105 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_106
	EXCEPT_USER_DEFINED_106 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_107
	EXCEPT_USER_DEFINED_107 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_108
	EXCEPT_USER_DEFINED_108 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_109
	EXCEPT_USER_DEFINED_109 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_110
	EXCEPT_USER_DEFINED_110 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_111
	EXCEPT_USER_DEFINED_111 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_112
	EXCEPT_USER_DEFINED_112 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_113
	EXCEPT_USER_DEFINED_113 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_114
	EXCEPT_USER_DEFINED_114 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_115
	EXCEPT_USER_DEFINED_115 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_116
	EXCEPT_USER_DEFINED_116 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_117
	EXCEPT_USER_DEFINED_117 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_118
	EXCEPT_USER_DEFINED_118 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_119
	EXCEPT_USER_DEFINED_119 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_120
	EXCEPT_USER_DEFINED_120 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_121
	EXCEPT_USER_DEFINED_121 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_122
	EXCEPT_USER_DEFINED_122 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_123
	EXCEPT_USER_DEFINED_123 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_124
	EXCEPT_USER_DEFINED_124 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_125
	EXCEPT_USER_DEFINED_125 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_126
	EXCEPT_USER_DEFINED_126 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_127
	EXCEPT_USER_DEFINED_127 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_128
	EXCEPT_USER_DEFINED_128 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_129
	EXCEPT_USER_DEFINED_129 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_130
	EXCEPT_USER_DEFINED_130 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_131
	EXCEPT_USER_DEFINED_131 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_132
	EXCEPT_USER_DEFINED_132 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_133
	EXCEPT_USER_DEFINED_133 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_134
	EXCEPT_USER_DEFINED_134 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_135
	EXCEPT_USER_DEFINED_135 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_136
	EXCEPT_USER_DEFINED_136 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_137
	EXCEPT_USER_DEFINED_137 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_138
	EXCEPT_USER_DEFINED_138 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_139
	EXCEPT_USER_DEFINED_139 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_140
	EXCEPT_USER_DEFINED_140 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_141
	EXCEPT_USER_DEFINED_141 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_142
	EXCEPT_USER_DEFINED_142 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_143
	EXCEPT_USER_DEFINED_143 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_144
	EXCEPT_USER_DEFINED_144 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_145
	EXCEPT_USER_DEFINED_145 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_146
	EXCEPT_USER_DEFINED_146 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_147
	EXCEPT_USER_DEFINED_147 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_148
	EXCEPT_USER_DEFINED_148 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_149
	EXCEPT_USER_DEFINED_149 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_150
	EXCEPT_USER_DEFINED_150 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_151
	EXCEPT_USER_DEFINED_151 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_152
	EXCEPT_USER_DEFINED_152 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_153
	EXCEPT_USER_DEFINED_153 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_154
	EXCEPT_USER_DEFINED_154 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_155
	EXCEPT_USER_DEFINED_155 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_156
	EXCEPT_USER_DEFINED_156 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_157
	EXCEPT_USER_DEFINED_157 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_158
	EXCEPT_USER_DEFINED_158 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_159
	EXCEPT_USER_DEFINED_159 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_160
	EXCEPT_USER_DEFINED_160 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_161
	EXCEPT_USER_DEFINED_161 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_162
	EXCEPT_USER_DEFINED_162 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_163
	EXCEPT_USER_DEFINED_163 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_164
	EXCEPT_USER_DEFINED_164 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_165
	EXCEPT_USER_DEFINED_165 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_166
	EXCEPT_USER_DEFINED_166 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_167
	EXCEPT_USER_DEFINED_167 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_168
	EXCEPT_USER_DEFINED_168 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_169
	EXCEPT_USER_DEFINED_169 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_170
	EXCEPT_USER_DEFINED_170 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_171
	EXCEPT_USER_DEFINED_171 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_172
	EXCEPT_USER_DEFINED_172 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_173
	EXCEPT_USER_DEFINED_173 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_174
	EXCEPT_USER_DEFINED_174 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_175
	EXCEPT_USER_DEFINED_175 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_176
	EXCEPT_USER_DEFINED_176 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_177
	EXCEPT_USER_DEFINED_177 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_178
	EXCEPT_USER_DEFINED_178 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_179
	EXCEPT_USER_DEFINED_179 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_180
	EXCEPT_USER_DEFINED_180 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_181
	EXCEPT_USER_DEFINED_181 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_182
	EXCEPT_USER_DEFINED_182 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_183
	EXCEPT_USER_DEFINED_183 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_184
	EXCEPT_USER_DEFINED_184 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_185
	EXCEPT_USER_DEFINED_185 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_186
	EXCEPT_USER_DEFINED_186 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_187
	EXCEPT_USER_DEFINED_187 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_188
	EXCEPT_USER_DEFINED_188 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_189
	EXCEPT_USER_DEFINED_189 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_190
	EXCEPT_USER_DEFINED_190 = EXCEPT_RESET_PC
.endif
.ifndef EXCEPT_USER_DEFINED_191
	EXCEPT_USER_DEFINED_191 = EXCEPT_RESET_PC
.endif



# ###########################################################################
# #                                                                         #
# #                         Exception Vector Table                          #
# #                                                                         #
# ###########################################################################

		.org 0x00000000
# Assigned vectors
###########################################################################
* 0x0000
dc.l		EXCEPT_RESET_SSP			/* Reset Initial Interrupt Stack Pointer */
dc.l		EXCEPT_RESET_PC				/* Reset Initial Program Counter */
dc.l		EXCEPT_ACCESS_ERROR			/* Bus Access Error */
dc.l		EXCEPT_ADDRESS_ERROR			/* Address Error */
* 0x0010
dc.l		EXCEPT_ILLEGAL_INSTRUCTION		/* Illegal Instruction */
dc.l		EXCEPT_DIVIDE_BY_ZERO			/* Integer Divide by Zero */
dc.l		EXCEPT_CHK_INSTRUCTION			/* CHK, CHK2 Instruction */
dc.l		EXCEPT_TRAPV_INSTRUCTION		/* FTRAPcc, TRAPcc, TRAPV Instructions */
* 0x0020
dc.l		EXCEPT_PRIVILEGE_VIOLATION		/* Privilege Violation */
dc.l		EXCEPT_TRACE				/* Trace */
dc.l		EXCEPT_LINE_A_EMULATOR			/* Line 1010 Emulator (Unimplemented A- Line Opcode) */
dc.l		EXCEPT_LINE_F_EMULATOR			/* Line 1111 Emulator (Unimplemented F-Line Opcode) */
* 0x0030
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_COPROCESSOR_VIOLATION		/* Coprocessor Protocol Violation */
dc.l		EXCEPT_FORMAT_ERROR			/* Format Error */
dc.l		EXCEPT_UNINITIALEZED_INTERRUPT		/* Uninitialized Interrupt */
* 0x0040
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
* 0x0050
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
* 0x0060
dc.l		EXCEPT_SUPRIOUS_INTERRUPT		/* Spurious Interrupt */
dc.l		EXCEPT_INTERRUPT_LEVEL1			/* Level 1 Interrupt Autovector */
dc.l		EXCEPT_INTERRUPT_LEVEL2			/* Level 2 Interrupt Autovector */
dc.l		EXCEPT_INTERRUPT_LEVEL3			/* Level 3 Interrupt Autovector */
* 0x0070
dc.l		EXCEPT_INTERRUPT_LEVEL4			/* Level 4 Interrupt Autovector */
dc.l		EXCEPT_INTERRUPT_LEVEL5			/* Level 5 Interrupt Autovector */
dc.l		EXCEPT_INTERRUPT_LEVEL6			/* Level 6 Interrupt Autovector */
dc.l		EXCEPT_INTERRUPT_LEVEL7			/* Level 7 Interrupt Autovector */
* 0x0080
dc.l		EXCEPT_TRAP0_INSTRUCTION		/* TRAP 0 Instruction Vector */
dc.l		EXCEPT_TRAP1_INSTRUCTION		/* TRAP 1 Instruction Vector */
dc.l		EXCEPT_TRAP2_INSTRUCTION		/* TRAP 2 Instruction Vector */
dc.l		EXCEPT_TRAP3_INSTRUCTION		/* TRAP 3 Instruction Vector */
* 0x0090
dc.l		EXCEPT_TRAP4_INSTRUCTION		/* TRAP 4 Instruction Vector */
dc.l		EXCEPT_TRAP5_INSTRUCTION		/* TRAP 5 Instruction Vector */
dc.l		EXCEPT_TRAP6_INSTRUCTION		/* TRAP 6 Instruction Vector */
dc.l		EXCEPT_TRAP7_INSTRUCTION		/* TRAP 7 Instruction Vector */
* 0x00a0
dc.l		EXCEPT_TRAP8_INSTRUCTION		/* TRAP 8 Instruction Vector */
dc.l		EXCEPT_TRAP9_INSTRUCTION		/* TRAP 9 Instruction Vector */
dc.l		EXCEPT_TRAP10_INSTRUCTION		/* TRAP 10 Instruction Vector */
dc.l		EXCEPT_TRAP11_INSTRUCTION		/* TRAP 11 Instruction Vector */
* 0x00b0
dc.l		EXCEPT_TRAP12_INSTRUCTION		/* TRAP 12 Instruction Vector */
dc.l		EXCEPT_TRAP13_INSTRUCTION		/* TRAP 13 Instruction Vector */
dc.l		EXCEPT_TRAP14_INSTRUCTION		/* TRAP 14 Instruction Vector */
dc.l		EXCEPT_TRAP15_INSTRUCTION		/* TRAP 15 Instruction Vector */
* 0x00c0
dc.l		EXCEPT_FP_BRANCH_SET			/* FP Branch or Set on Unordered Condition */
dc.l		EXCEPT_FP_INEXACT_RESULT		/* FP Inexact Result */
dc.l		EXCEPT_FP_DIVIDE_BY_ZERO		/* FP Divide by Zero  */
dc.l		EXCEPT_FP_UNDERFLOW			/* FP Underflow */
* 0x00d0
dc.l		EXCEPT_FP_OPERAND_ERROR			/* FP Operand Error */
dc.l		EXCEPT_FP_OVERFLOW			/* FP Overflow */
dc.l		EXCEPT_FP_SIGNALING_NAN			/* FP Signaling NAN */
dc.l		EXCEPT_FP_UNIMPLEMENTED_DATA_TYPE	/* FP Unimplemented Data Type */
* 0x00e0
dc.l		EXCEPT_MMU_CONFIGURATION_ERROR		/* MMU Configuration Error */
dc.l		EXCEPT_MMU_ILLEGAL_OPERATION		/* MMU Illegal Operation Error */
dc.l		EXCEPT_MMU_ACCESS_LEVEL_VIOLATION	/* MMU Access Level Violation Error */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
* 0x00f0
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */
dc.l		EXCEPT_RESERVED				/* Unassigned, Reserved */

# User defined vectors
###########################################################################
* 0x0100
dc.l		EXCEPT_USER_DEFINED_0				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_1				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_2				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_3				/* User Defined Vector */
* 0x0110
dc.l		EXCEPT_USER_DEFINED_4				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_5				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_6				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_7				/* User Defined Vector */
* 0x0120
dc.l		EXCEPT_USER_DEFINED_8				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_9				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_10				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_11				/* User Defined Vector */
* 0x0130
dc.l		EXCEPT_USER_DEFINED_12				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_13				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_14				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_15				/* User Defined Vector */
* 0x0140
dc.l		EXCEPT_USER_DEFINED_16				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_17				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_18				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_19				/* User Defined Vector */
* 0x0150
dc.l		EXCEPT_USER_DEFINED_20				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_21				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_22				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_23				/* User Defined Vector */
* 0x0160
dc.l		EXCEPT_USER_DEFINED_24				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_25				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_26				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_27				/* User Defined Vector */
* 0x0170
dc.l		EXCEPT_USER_DEFINED_28				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_29				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_30				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_31				/* User Defined Vector */
* 0x0180
dc.l		EXCEPT_USER_DEFINED_32				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_33				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_34				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_35				/* User Defined Vector */
* 0x0190
dc.l		EXCEPT_USER_DEFINED_36				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_37				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_38				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_39				/* User Defined Vector */
* 0x01A0
dc.l		EXCEPT_USER_DEFINED_40				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_41				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_42				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_43				/* User Defined Vector */
* 0x01B0
dc.l		EXCEPT_USER_DEFINED_44				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_45				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_46				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_47				/* User Defined Vector */
* 0x01C0
dc.l		EXCEPT_USER_DEFINED_48				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_49				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_50				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_51				/* User Defined Vector */
* 0x01D0
dc.l		EXCEPT_USER_DEFINED_52				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_53				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_54				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_55				/* User Defined Vector */
* 0x01E0
dc.l		EXCEPT_USER_DEFINED_56				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_57				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_58				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_59				/* User Defined Vector */
* 0x01F0
dc.l		EXCEPT_USER_DEFINED_60				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_61				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_62				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_63				/* User Defined Vector */
* 0x0200
dc.l		EXCEPT_USER_DEFINED_64				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_65				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_66				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_67				/* User Defined Vector */
* 0x0210
dc.l		EXCEPT_USER_DEFINED_68				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_69				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_70				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_71				/* User Defined Vector */
* 0x0220
dc.l		EXCEPT_USER_DEFINED_72				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_73				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_74				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_75				/* User Defined Vector */
* 0x0230
dc.l		EXCEPT_USER_DEFINED_76				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_77				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_78				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_79				/* User Defined Vector */
* 0x0240
dc.l		EXCEPT_USER_DEFINED_80				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_81				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_82				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_83				/* User Defined Vector */
* 0x0250
dc.l		EXCEPT_USER_DEFINED_84				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_85				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_86				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_87				/* User Defined Vector */
* 0x0260
dc.l		EXCEPT_USER_DEFINED_88				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_89				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_90				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_91				/* User Defined Vector */
* 0x0270
dc.l		EXCEPT_USER_DEFINED_92				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_93				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_94				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_95				/* User Defined Vector */
* 0x0280
dc.l		EXCEPT_USER_DEFINED_96				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_97				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_98				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_99				/* User Defined Vector */
* 0x0290
dc.l		EXCEPT_USER_DEFINED_100				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_101				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_102				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_103				/* User Defined Vector */
* 0x02A0
dc.l		EXCEPT_USER_DEFINED_104				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_105				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_106				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_107				/* User Defined Vector */
* 0x02B0
dc.l		EXCEPT_USER_DEFINED_108				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_109				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_110				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_111				/* User Defined Vector */
* 0x02C0
dc.l		EXCEPT_USER_DEFINED_112				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_113				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_114				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_115				/* User Defined Vector */
* 0x02D0
dc.l		EXCEPT_USER_DEFINED_116				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_117				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_118				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_119				/* User Defined Vector */
* 0x02E0
dc.l		EXCEPT_USER_DEFINED_120				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_121				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_122				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_123				/* User Defined Vector */
* 0x02F0
dc.l		EXCEPT_USER_DEFINED_124				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_125				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_126				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_127				/* User Defined Vector */
* 0x0300
dc.l		EXCEPT_USER_DEFINED_128				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_129				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_130				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_131				/* User Defined Vector */
* 0x0310
dc.l		EXCEPT_USER_DEFINED_132				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_133				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_134				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_135				/* User Defined Vector */
* 0x0320
dc.l		EXCEPT_USER_DEFINED_136				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_137				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_138				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_139				/* User Defined Vector */
* 0x0330
dc.l		EXCEPT_USER_DEFINED_140				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_141				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_142				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_143				/* User Defined Vector */
* 0x0340
dc.l		EXCEPT_USER_DEFINED_144				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_145				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_146				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_147				/* User Defined Vector */
* 0x0350
dc.l		EXCEPT_USER_DEFINED_148				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_149				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_150				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_151				/* User Defined Vector */
* 0x0360
dc.l		EXCEPT_USER_DEFINED_152				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_153				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_154				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_155				/* User Defined Vector */
* 0x0370
dc.l		EXCEPT_USER_DEFINED_156				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_157				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_158				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_159				/* User Defined Vector */
* 0x0380
dc.l		EXCEPT_USER_DEFINED_160				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_161				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_162				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_163				/* User Defined Vector */
* 0x0390
dc.l		EXCEPT_USER_DEFINED_164				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_165				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_166				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_167				/* User Defined Vector */
* 0x03A0
dc.l		EXCEPT_USER_DEFINED_168				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_169				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_170				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_171				/* User Defined Vector */
* 0x03B0
dc.l		EXCEPT_USER_DEFINED_172				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_173				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_174				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_175				/* User Defined Vector */
* 0x03C0
dc.l		EXCEPT_USER_DEFINED_176				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_177				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_178				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_179				/* User Defined Vector */
* 0x03D0
dc.l		EXCEPT_USER_DEFINED_180				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_181				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_182				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_183				/* User Defined Vector */
* 0x03E0
dc.l		EXCEPT_USER_DEFINED_184				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_185				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_186				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_187				/* User Defined Vector */
* 0x03F0
dc.l		EXCEPT_USER_DEFINED_188				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_189				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_190				/* User Defined Vector */
dc.l		EXCEPT_USER_DEFINED_191				/* User Defined Vector */
