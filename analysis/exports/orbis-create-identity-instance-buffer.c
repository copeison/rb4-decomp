// Creates and uploads the named IdentityInstanceVBuffer.
__int64 __fastcall orbis_create_identity_instance_buffer(__int64 a1)
{
  int v3; // eax
  __m256 v14; // [rsp+0h] [rbp-90h]
  _BYTE v15[88]; // [rsp+20h] [rbp-70h] BYREF
  __int64 v16; // [rsp+78h] [rbp-18h]

  v16 = 0x6365786562696C2FLL;
  if ( byte_1ADEF58 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADEF58);
    if ( v3 != 0 )
    {
      qword_1ADEF50 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADEF58);
    }
  }
  sub_37A920(qword_1ADEF50);
  *(_QWORD *)(a1 + 4136) = sub_37AE70(120, "IdentityInstanceVBuffer", 4);
  sub_37A9B0();
  sub_8E1BD0(a1 + 3992, *(_QWORD *)(a1 + 4136), 1);
  _RAX = &unk_1B5D268;
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups [rbp+var_70+10h], ymm0 }
  *(_QWORD *)&v15[48] = 0;
  __asm
  {
    vmovups xmm0, xmmword ptr [rax]
    vmovups xmm1, xmmword ptr [rax]
  }
  __asm
  {
    vmovups xmmword ptr [rbp+var_38], xmm0
    vmovups xmmword ptr [rbp+var_38+10h], xmm1
  }
  *(_QWORD *)v14.m256_f32 = __PAIR64__(unk_19E6678, dword_19E666C);
  *(_QWORD *)&v14.m256_f32[4] = __PAIR64__(unk_19E667C, unk_19E6670);
  *(_DWORD *)v15 = unk_19E6674;
  *(_DWORD *)&v15[4] = unk_19E6680;
  *(_QWORD *)&v14.m256_f32[2] = __PAIR64__(dword_19E668C[1], unk_19E6684);
  *(_QWORD *)&v14.m256_f32[6] = __PAIR64__(dword_19E668C[2], unk_19E6688);
  *(_DWORD *)&v15[8] = dword_19E668C[0];
  *(_DWORD *)&v15[12] = dword_19E668C[3];
  sub_215340(&dword_19E666C, &v15[16], 0);
  _RAX = *(_QWORD *)(a1 + 4136);
  __asm
  {
    vmovups ymm0, [rbp+var_90]
    vmovups ymm1, [rbp+var_70]
    vmovups ymm2, ymmword ptr [rbp-50h]
    vmovups ymm3, [rbp+var_38]
    vmovups ymmword ptr [rax+58h], ymm3
    vmovups ymmword ptr [rax+40h], ymm2
    vmovups ymmword ptr [rax+20h], ymm1
    vmovups ymmword ptr [rax], ymm0
  }
  return 0x6365786562696C2FLL;
}
