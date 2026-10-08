// Creates and uploads the named DefaultVBuffer.
__int64 __fastcall orbis_create_default_vertex_buffer(__int64 a1)
{
  __int64 v2; // r14
  int v3; // eax
  _BYTE v10[4]; // [rsp+Ch] [rbp-24h] BYREF
  __int64 v11; // [rsp+10h] [rbp-20h]

  v11 = 0x6365786562696C2FLL;
  v2 = sub_4435E0(3);
  if ( byte_1ADEF48 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADEF48);
    if ( v3 != 0 )
    {
      qword_1ADEF40 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADEF48);
    }
  }
  sub_37A920(qword_1ADEF40);
  *(_QWORD *)(a1 + 3984) = sub_37AE70(*(_QWORD *)(v2 + 8), "DefaultVBuffer", 4);
  sub_37A9B0();
  _RAX = *(_QWORD *)(a1 + 3984);
  _RCX = &unk_1A73E50;
  *(_DWORD *)(_RAX + 96) = unk_1A73EB0;
  __asm
  {
    vmovups ymm0, ymmword ptr [rcx]
    vmovups ymm1, ymmword ptr [rcx+20h]
    vmovups ymm2, ymmword ptr [rcx+40h]
  }
  __asm
  {
    vmovups ymmword ptr [rax+40h], ymm2
    vmovups ymmword ptr [rax+20h], ymm1
    vmovups ymmword ptr [rax], ymm0
  }
  sub_8E1840(a1 + 3852, *(_QWORD *)(a1 + 3984), v10, 1, v2);
  return 0x6365786562696C2FLL;
}
