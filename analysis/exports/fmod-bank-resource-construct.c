__int64 __fastcall fmod_bank_resource_construct(volatile __int32 *a1)
{
  _QWORD *v6; // rax
  char v8[8]; // [rsp+0h] [rbp-30h] BYREF
  __int64 v9; // [rsp+8h] [rbp-28h]

  _RBX = a1;
  v9 = 0x6365786562696C2FLL;
  *(_QWORD *)a1 = &unk_18E8A18;
  *((_QWORD *)a1 + 1) = 19246190;
  *(double *)&_XMM0 = sub_1AF950(a1 + 2, &byte_125D142);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  _InterlockedExchange(_RBX + 4, 0);
  *((_WORD *)_RBX + 10) = 0;
  __asm { vmovups xmmword ptr [rbx+18h], xmm0 }
  *((_DWORD *)_RBX + 10) = 0;
  *(_QWORD *)_RBX = &vtable_FModBankResource;
  nullsub_18(v8, "EASTL map");
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [rbx+38h], ymm0 }
  *((_QWORD *)_RBX + 11) = 0;
  nullsub_19(_RBX + 24, v8);
  *((_QWORD *)_RBX + 7) = _RBX + 14;
  *((_QWORD *)_RBX + 8) = _RBX + 14;
  *((_QWORD *)_RBX + 9) = 0;
  *((_BYTE *)_RBX + 80) = 0;
  *((_QWORD *)_RBX + 11) = 0;
  *((_QWORD *)_RBX + 13) = 0;
  *((_BYTE *)_RBX + 112) = 0;
  scePthreadMutexLock(&unk_19F2E48);
  ++dword_19F2E40;
  v6 = (_QWORD *)sub_252CF0(&unk_19F2E68, 24, 0);
  v6[2] = _RBX;
  *v6 = &xmmword_19F2E50;
  v6[1] = *((_QWORD *)&xmmword_19F2E50 + 1);
  **((_QWORD **)&xmmword_19F2E50 + 1) = v6;
  *((_QWORD *)&xmmword_19F2E50 + 1) = v6;
  ++qword_19F2E60;
  --dword_19F2E40;
  scePthreadMutexUnlock(&unk_19F2E48);
  return 0x6365786562696C2FLL;
}
