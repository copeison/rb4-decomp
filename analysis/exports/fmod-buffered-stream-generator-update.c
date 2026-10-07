__int64 __fastcall fmod_buffered_stream_generator_update(__int64 a1)
{
  unsigned int v1; // r14d

  _RBX = a1;
  if ( *(_DWORD *)(a1 + 28) == 5 )
    return 0;
  if ( (*(unsigned __int8 (__fastcall **)(_QWORD))(**(_QWORD **)(a1 + 312) + 176LL))(*(_QWORD *)(a1 + 312)) == 0 )
  {
LABEL_6:
    *(_DWORD *)(_RBX + 28) = 5;
    return 0;
  }
  if ( *(_DWORD *)(_RBX + 28) == 6 )
  {
    LOBYTE(v1) = 1;
    if ( (*(unsigned int (__fastcall **)(_QWORD))(**(_QWORD **)(_RBX + 312) + 24LL))(*(_QWORD *)(_RBX + 312)) != 5 )
      return v1;
    goto LABEL_6;
  }
  if ( *(_BYTE *)(_RBX + 480) != 0 )
    fmod_buffered_stream_generator_finish_sound_open(_RBX);
  LOBYTE(v1) = 1;
  if ( *(_BYTE *)(_RBX + 525) != 0 )
  {
    (*(void (__fastcall **)(__int64))(*(_QWORD *)(_RBX + 80) + 40LL))(_RBX + 80);
    *(double *)&_XMM0 = sub_25AC70(unk_19F2520 + 240LL);
    __asm
    {
      vsubss  xmm1, xmm0, dword ptr [rbx+230h]
      vmovss  dword ptr [rbx+230h], xmm0
    }
    *(_DWORD *)(_RBX + 532) = *(_DWORD *)(_RBX + 528);
    __asm
    {
      vmovss  xmm2, dword ptr [rbx+1D4h]
      vmulss  xmm3, xmm2, cs:dword_125CD48
      vmulss  xmm2, xmm2, cs:dword_125CD4C
      vcvtsi2ss xmm0, xmm0, ecx
      vdivss  xmm0, xmm0, xmm1
      vmaxss  xmm3, xmm3, xmm0
      vcmpltss xmm0, xmm2, xmm0
      vblendvps xmm0, xmm3, xmm2, xmm0
      vmovss  dword ptr [rbx+218h], xmm0
      vmulss  xmm1, xmm1, dword ptr [rbx+234h]
    }
    sub_1172A40(_RBX + 540, *(double *)&_XMM0, *(double *)&_XMM1);
    (*(void (__fastcall **)(__int64))(*(_QWORD *)(_RBX + 80) + 56LL))(_RBX + 80);
  }
  return v1;
}
