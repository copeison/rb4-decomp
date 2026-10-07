// local variable allocation has failed, the output may be wrong!
void __fastcall special_pad_reader_append_calibration_samples(void *self, const void *pad_data, int count)
{
  __int64 v10; // rdx
  char *v11; // rsi

  if ( *((_DWORD *)self + 2049) != 0 && count > 0 )
  {
    LODWORD(_RAX) = *((_DWORD *)self + 2116);
    __asm
    {
      vmovss  xmm0, cs:dword_12D127C
      vmovss  xmm1, cs:dword_12D1280
    }
    _R8 = (char *)self + 8208;
    _R9 = (char *)self + 8212;
    v10 = (unsigned int)count;
    v11 = (char *)pad_data + 117;
    __asm { vxorps  xmm2, xmm2, xmm2 }
    do
    {
      if ( (int)_RAX >= 64 )
      {
        __asm { vmovups ymm3, ymmword ptr [r9+0DCh] }
        LODWORD(_RAX) = 63;
        __asm
        {
          vmovups ymmword ptr [r8+0DCh], ymm3
          vmovups ymm3, ymmword ptr [r9+0C0h]
          vmovups ymmword ptr [r8+0C0h], ymm3
          vmovups ymm3, ymmword ptr [r9+0A0h]
          vmovups ymmword ptr [r8+0A0h], ymm3
          vmovups ymm3, ymmword ptr [r9+80h]
          vmovups ymmword ptr [r8+80h], ymm3
          vmovups ymm3, ymmword ptr [r9]
          vmovups ymm4, ymmword ptr [r9+20h]
          vmovups ymm5, ymmword ptr [r9+40h]
          vmovups ymm6, ymmword ptr [r9+60h]
          vmovups ymmword ptr [r8+60h], ymm6
          vmovups ymmword ptr [r8+40h], ymm5
          vmovups ymmword ptr [r8+20h], ymm4
          vmovups ymmword ptr [r8], ymm3
        }
        *((_DWORD *)self + 2116) = 63;
      }
      if ( *((_DWORD *)self + 2049) == 1 )
      {
        __asm { vcvtsi2ss xmm3, xmm7, ecx }
      }
      else
      {
        __asm
        {
          vcvtsi2ss xmm3, xmm7, ecx
          vrsqrtss xmm4, xmm3, xmm3
          vmulss  xmm5, xmm3, xmm4
          vcmpeqss xmm3, xmm3, xmm2
          vmulss  xmm4, xmm5, xmm4
          vmulss  xmm6, xmm5, xmm0
          vaddss  xmm4, xmm4, xmm1
          vmulss  xmm4, xmm6, xmm4
          vandnps xmm3, xmm3, xmm4
        }
      }
      _RAX = (int)_RAX;
      v11 += 120;
      __asm { vmovss  dword ptr [rdi+rax*4+2010h], xmm3 }
      LODWORD(_RAX) = *((_DWORD *)self + 2116) + 1;
      --*(_QWORD *)&count;
      *((_DWORD *)self + 2116) = _RAX;
    }
    while ( *(_QWORD *)&count != 0 );
  }
}
