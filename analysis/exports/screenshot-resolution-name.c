// Formats the current or fixed screenshot resolution mode.
const char *__fastcall screenshot_resolution_name(
        int a1,
        __m128 _XMM0,
        __m128 _XMM1,
        __int64 a4,
        __int64 *a5,
        __int64 *a6,
        int a7,
        int a8)
{
  unsigned int v8; // r15d
  unsigned int v9; // r14d
  __int64 v22; // rax
  __int64 v23; // rbx
  __int64 v24; // rax
  __int64 v25; // rax
  __int64 v26; // rax
  __m256 v28; // [rsp+0h] [rbp-A8h]
  _BYTE v29[48]; // [rsp+30h] [rbp-78h] BYREF
  __int64 v30; // [rsp+60h] [rbp-48h]

  v30 = 0x6365786562696C2FLL;
  if ( a1 == 0 )
    return "Window Dimensions";
  v8 = -1;
  v9 = -1;
  if ( (unsigned int)a1 <= 5 )
  {
    a6 = qword_1281700;
    a5 = qword_1281720;
    v8 = *((_DWORD *)qword_1281700 + a1);
    v9 = *((_DWORD *)qword_1281720 + a1);
  }
  __asm
  {
    vcvtsi2ss xmm0, xmm0, r15d
    vcvtsi2ss xmm1, xmm1, r14d
    vdivss  xmm0, xmm0, xmm1
    vmovups ymm1, cs:ymmword_1281640
    vmovups [rsp+0A8h+var_A8], ymm1
    vaddss  xmm1, xmm0, cs:dword_1281660
    vandps  xmm1, xmm1, cs:xmmword_1281680
    vucomiss xmm1, cs:dword_1281664
  }
  if ( (unsigned int)a1 <= 5 )
  {
    sub_2472C0(v29, "%d x %d (%d:%d)", (_DWORD)a5, (_DWORD)a6, a7, a8);
    v24 = sub_247CD0(v29, v8);
    v25 = sub_247CD0(v24, v9);
    v26 = sub_247CD0(v25, LODWORD(v28.m256_f32[0]));
    sub_247CD0(v26, LODWORD(v28.m256_f32[1]));
    v23 = sub_2484E0(v29);
    sub_247510(v29);
  }
  else
  {
    __asm
    {
      vaddss  xmm1, xmm0, cs:dword_1281668
      vandps  xmm1, xmm1, cs:xmmword_1281680
      vucomiss xmm1, cs:dword_1281664
    }
    __asm
    {
      vaddss  xmm1, xmm0, cs:dword_128166C
      vandps  xmm1, xmm1, cs:xmmword_1281680
      vucomiss xmm1, cs:dword_1281664
    }
    __asm
    {
      vaddss  xmm0, xmm0, cs:dword_1281670
      vandps  xmm0, xmm0, cs:xmmword_1281680
      vucomiss xmm0, cs:dword_1281664
    }
    sub_2472C0(v29, "%d x %d", (_DWORD)a5, (_DWORD)a6);
    v22 = sub_247CD0(v29, v8);
    sub_247CD0(v22, v9);
    v23 = sub_2484E0(v29);
    sub_247510(v29);
  }
  return (const char *)v23;
}
