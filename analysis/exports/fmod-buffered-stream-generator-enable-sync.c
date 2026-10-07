__int128 __usercall fmod_buffered_stream_generator_enable_sync@<xmm0>(
        __int64 a1@<rdi>,
        __m128 _XMM0@<xmm0>,
        __m128 _XMM1@<xmm1>)
{
  __int64 v4; // r14
  __int128 result; // xmm0

  _RBX = a1;
  __asm
  {
    vmovss  [rbp+var_1C], xmm1
    vmovss  [rbp+var_18], xmm0
  }
  v4 = a1 + 80;
  (*(void (__fastcall **)(__int64))(*(_QWORD *)(a1 + 80) + 40LL))(a1 + 80);
  *(_BYTE *)(_RBX + 525) = 1;
  __asm { vmovss  xmm2, cs:dword_125CDF0 }
  __asm
  {
    vmovss  xmm1, dword ptr [rbx+1E4h]
    vmovss  xmm0, dword ptr [rbx+1D4h]
    vmulss  xmm3, xmm1, xmm2
    vmulss  xmm3, xmm3, xmm0
    vcvttss2si eax, xmm3
  }
  *(_DWORD *)(_RBX + 532) = _EAX;
  __asm
  {
    vaddss  xmm3, xmm1, dword ptr [rax]
    vmulss  xmm1, xmm2, xmm0
    vmulss  xmm1, xmm3, xmm1
    vmovss  [rbp+var_14], xmm3
    vcvttss2si eax, xmm1
  }
  *(_DWORD *)(_RBX + 528) = _EAX;
  sub_1172AD0(_RBX + 540, 0);
  *(_DWORD *)(_RBX + 536) = 0;
  *(double *)&_XMM0 = sub_25AC70(unk_19F2520 + 240LL);
  __asm { vmovss  dword ptr [rbx+230h], xmm0 }
  __asm
  {
    vmovss  xmm0, [rbp+var_18]
    vmovss  dword ptr [rbx+228h], xmm0
  }
  *(_DWORD *)(_RBX + 556) = 0;
  __asm
  {
    vmovss  xmm0, [rbp+var_1C]
    vmovss  dword ptr [rbx+234h], xmm0
  }
  (*(void (__fastcall **)(__int64))(*(_QWORD *)(_RBX + 80) + 56LL))(v4);
  __asm { vmovss  xmm0, [rbp+var_14] }
  return result;
}
