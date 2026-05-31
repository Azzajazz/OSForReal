.macro exc_start_with_err_code iv
.global exc_start_\iv
.type exc_start_\iv, @function
/*
Start point for exceptions. This stub is responsible for saving and restoring registers
when an interrupt is called.

Params: @NOTE: These params are pushed automatically by the CPU.
    [esp]: interrupt error code
    [esp + 4]: old eip
    [esp + 8]: old cs
    [esp + 12]: old eflags
    [esp + 16]: old esp
    [esp + 20]: old ss
Returns: None
*/
exc_start_\iv:
    push %eax
    push %ecx
    push %edx
    push %ebx
    push %ebp
    push %esi
    push %edi

    /* Call the C exception handler. */
    push %esp
    push $\iv
    call c_exception_handler
    add $8, %esp

    pop %edi
    pop %esi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax
    iret
.size exc_start_\iv, . - exc_start_\iv
.endm

.macro exc_start iv
.global exc_start_\iv
.type exc_start_\iv, @function
/*
Start point for exceptions. This stub is responsible for saving and restoring registers
when an interrupt is called.

Params: @NOTE: These params are pushed automatically by the CPU.
    [esp + 4]: old eip
    [esp + 8]: old cs
    [esp + 12]: old eflags
    [esp + 16]: old esp
    [esp + 20]: old ss
Returns: None
*/
exc_start_\iv:
    /*
    Higher level interrupt handlers expect an error code. In case there isn't one,
    just push 0 to fill the slot.
    */
    push $0

    push %eax
    push %ecx
    push %edx
    push %ebx
    push %ebp
    push %esi
    push %edi

    /* Call the C exception handler. */
    push %esp
    push $\iv
    call c_exception_handler
    add $8, %esp

    pop %edi
    pop %esi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax

    /* Get rid of the 0 we pushed to fill the error code slot. */
    add $4, %esp
    iret
.size exc_start_\iv, . - exc_start_\iv
.endm

.macro int_start iv
.global int_start_\iv
.type int_start_\iv, @function
/*
Start point for interrupts. This stub is responsible for saving and restoring registers
when an interrupt is called.

Params: @NOTE: These params are pushed automatically by the CPU.
    [esp + 4]: old eip
    [esp + 8]: old cs
    [esp + 12]: old eflags
    [esp + 16]: old esp
    [esp + 20]: old ss
Returns: None
*/
int_start_\iv:
    push %eax
    push %ecx
    push %edx
    push %ebx
    push %ebp
    push %esi
    push %edi

    /* Call the C interrupt dispatch. */
    push %esp
    push $\iv
    call c_interrupt_dispatch
    add $8, %esp

    pop %edi
    pop %esi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax
    iret
.size int_start_\iv, . - int_start_\iv
.endm

exc_start 0
exc_start 1
exc_start 2
exc_start 3
exc_start 4
exc_start 5
exc_start 6
exc_start 7
exc_start_with_err_code 8
exc_start 9
exc_start 10
exc_start_with_err_code 11
exc_start_with_err_code 12
exc_start_with_err_code 13
exc_start_with_err_code 14
exc_start 15
exc_start 16
exc_start_with_err_code 17
exc_start 18
exc_start 19
exc_start 20
exc_start_with_err_code 21
exc_start 22
exc_start 23
exc_start 24
exc_start 25
exc_start 26
exc_start 27
exc_start 28
exc_start_with_err_code 29
exc_start_with_err_code 30
exc_start 31
int_start 32
int_start 33
int_start 34
int_start 35
int_start 36
int_start 37
int_start 38
int_start 39
int_start 40
int_start 41
int_start 42
int_start 43
int_start 44
int_start 45
int_start 46
int_start 47
int_start 48
int_start 49
int_start 50
int_start 51
int_start 52
int_start 53
int_start 54
int_start 55
int_start 56
int_start 57
int_start 58
int_start 59
int_start 60
int_start 61
int_start 62
int_start 63
int_start 64
int_start 65
int_start 66
int_start 67
int_start 68
int_start 69
int_start 70
int_start 71
int_start 72
int_start 73
int_start 74
int_start 75
int_start 76
int_start 77
int_start 78
int_start 79
int_start 80
int_start 81
int_start 82
int_start 83
int_start 84
int_start 85
int_start 86
int_start 87
int_start 88
int_start 89
int_start 90
int_start 91
int_start 92
int_start 93
int_start 94
int_start 95
int_start 96
int_start 97
int_start 98
int_start 99
int_start 100
int_start 101
int_start 102
int_start 103
int_start 104
int_start 105
int_start 106
int_start 107
int_start 108
int_start 109
int_start 110
int_start 111
int_start 112
int_start 113
int_start 114
int_start 115
int_start 116
int_start 117
int_start 118
int_start 119
int_start 120
int_start 121
int_start 122
int_start 123
int_start 124
int_start 125
int_start 126
int_start 127
int_start 128
int_start 129
int_start 130
int_start 131
int_start 132
int_start 133
int_start 134
int_start 135
int_start 136
int_start 137
int_start 138
int_start 139
int_start 140
int_start 141
int_start 142
int_start 143
int_start 144
int_start 145
int_start 146
int_start 147
int_start 148
int_start 149
int_start 150
int_start 151
int_start 152
int_start 153
int_start 154
int_start 155
int_start 156
int_start 157
int_start 158
int_start 159
int_start 160
int_start 161
int_start 162
int_start 163
int_start 164
int_start 165
int_start 166
int_start 167
int_start 168
int_start 169
int_start 170
int_start 171
int_start 172
int_start 173
int_start 174
int_start 175
int_start 176
int_start 177
int_start 178
int_start 179
int_start 180
int_start 181
int_start 182
int_start 183
int_start 184
int_start 185
int_start 186
int_start 187
int_start 188
int_start 189
int_start 190
int_start 191
int_start 192
int_start 193
int_start 194
int_start 195
int_start 196
int_start 197
int_start 198
int_start 199
int_start 200
int_start 201
int_start 202
int_start 203
int_start 204
int_start 205
int_start 206
int_start 207
int_start 208
int_start 209
int_start 210
int_start 211
int_start 212
int_start 213
int_start 214
int_start 215
int_start 216
int_start 217
int_start 218
int_start 219
int_start 220
int_start 221
int_start 222
int_start 223
int_start 224
int_start 225
int_start 226
int_start 227
int_start 228
int_start 229
int_start 230
int_start 231
int_start 232
int_start 233
int_start 234
int_start 235
int_start 236
int_start 237
int_start 238
int_start 239
int_start 240
int_start 241
int_start 242
int_start 243
int_start 244
int_start 245
int_start 246
int_start 247
int_start 248
int_start 249
int_start 250
int_start 251
int_start 252
int_start 253
int_start 254
int_start 255
