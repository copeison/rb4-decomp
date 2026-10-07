__int128 __usercall fmod_buffered_stream_generator_compute_sync_adjustment@<xmm0>(__int64 _RDI@<rdi>)
{
  int v5; // esi
  __int128 result; // xmm0

  __asm { vmovss  xmm0, dword ptr [rdi+1E4h] }
  __asm { vmulss  xmm0, xmm0, cs:dword_125CDB8 }
  __asm
  {
    vmulss  xmm0, xmm0, dword ptr [rdi+1D4h]
    vcvttss2si eax, xmm0
  }
  v5 = _EAX - *(_DWORD *)(_RDI + 528);
  if ( v5 < 1 )
    v5 = *(_DWORD *)(_RDI + 528) - _EAX;
  if ( v5 <= 0 )
  {
    __asm { vxorps  xmm0, xmm0, xmm0 }
  }
  else
  {
    __asm
    {
      vmovss  xmm1, dword ptr [rdi+21Ch]
      vxorps  xmm0, xmm0, xmm0
      vucomiss xmm1, xmm0
    }
    __asm
    {
      vxorps  xmm0, xmm0, xmm0
      vucomiss xmm1, xmm0
    }
    if ( *(_DWORD *)(_RDI + 528) >= _EAX )
    {
      __asm
      {
        vmovsd  xmm0, qword ptr [rdi+0E8h]
        vcvtsd2ss xmm0, xmm0, xmm0
        vdivss  xmm0, xmm1, xmm0
      }
    }
  }
  return result;
}
