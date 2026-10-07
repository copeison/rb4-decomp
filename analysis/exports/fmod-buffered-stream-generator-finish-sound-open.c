__int64 __fastcall fmod_buffered_stream_generator_finish_sound_open(__int64 a1)
{
  __int64 v2; // rax
  __int64 *v3; // r13
  int v4; // ecx
  _DWORD *v5; // rax
  __int64 i; // rdx
  __int64 v11; // rbx
  __int64 j; // r13
  int v14; // [rsp+0h] [rbp-50h] BYREF
  _BYTE v15[4]; // [rsp+4h] [rbp-4Ch] BYREF
  int v16; // [rsp+8h] [rbp-48h] BYREF
  _BYTE v17[4]; // [rsp+Ch] [rbp-44h] BYREF
  _BYTE v18[6]; // [rsp+10h] [rbp-40h] BYREF
  char v19; // [rsp+16h] [rbp-3Ah] BYREF
  char v20; // [rsp+17h] [rbp-39h] BYREF
  int v21; // [rsp+18h] [rbp-38h] BYREF
  int v22; // [rsp+1Ch] [rbp-34h] BYREF
  __int64 v23; // [rsp+20h] [rbp-30h]

  _R15 = a1;
  v23 = 0x6365786562696C2FLL;
  v21 = 0;
  v20 = 0;
  v19 = 0;
  FMOD::Sound::getOpenState(*(_QWORD *)(a1 + 456), &v22, &v21, &v20, &v19);
  if ( v22 == 0 )
  {
    (*(void (__fastcall **)(__int64))(*(_QWORD *)(_R15 + 80) + 40LL))(_R15 + 80);
    FMOD::Sound::getFormat(*(_QWORD *)(_R15 + 456), v18, v17, &v16, v15);
    FMOD::Sound::getDefaults(*(FMOD::Sound **)(_R15 + 456), (float *)(_R15 + 468), &v14);
    FMOD::Sound::getLength(*(FMOD::Sound **)(_R15 + 456), (unsigned int *)(_R15 + 464), 2u);
    if ( v16 == 1 )
    {
      v2 = *(_QWORD *)(_R15 + 280);
      v3 = (__int64 *)(_R15 + 288);
      v4 = -1431655765 * ((*(_QWORD *)(_R15 + 288) - v2) >> 6);
      if ( v4 > 0 )
      {
        v5 = (_DWORD *)(v2 + 156);
        for ( i = 0; i < v4; ++i )
        {
          *v5 = 1;
          v5 += 48;
        }
      }
    }
    else
    {
      v3 = (__int64 *)(_R15 + 288);
    }
    __asm
    {
      vmovsd  xmm1, qword ptr [r15+0E8h]
      vmovss  xmm0, dword ptr [r15+1D4h]
    }
    __asm
    {
      vcvtsd2ss xmm1, xmm1, xmm1
      vdivss  xmm0, xmm0, xmm1
      vmovss  dword ptr [r15+1DCh], xmm0
    }
    FMOD::Sound::seekData(*(FMOD::Sound **)(_R15 + 456), 0);
    v11 = *(_QWORD *)(_R15 + 280);
    for ( j = *v3; j != v11; v11 += 192 )
      sub_264660(*(_QWORD *)(_R15 + 16) + 72LL, v11);
    *(_BYTE *)(_R15 + 480) = 0;
    (*(void (__fastcall **)(__int64))(*(_QWORD *)(_R15 + 80) + 56LL))(_R15 + 80);
  }
  return 0x6365786562696C2FLL;
}
