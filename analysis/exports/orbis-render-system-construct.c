// Constructs Orbis video, worker, synchronization, and deferred-command state over the base render system.
__int64 __fastcall orbis_render_system_construct(__int64 a1)
{
  int v4; // ecx
  int v5; // r8d
  int v6; // r9d
  _BYTE v12[8]; // [rsp+0h] [rbp-30h] BYREF
  __int64 v13; // [rsp+8h] [rbp-28h]

  _RBX = a1;
  v13 = 0x6365786562696C2FLL;
  render_system_construct(a1);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *(_QWORD *)_RBX = &unk_195ED70;
  __asm { vmovups xmmword ptr [rbx+0EE8h], xmm0 }
  *(_QWORD *)(_RBX + 3840) = 1;
  *(_QWORD *)(_RBX + 3984) = 0;
  __asm { vmovups xmmword ptr [rbx+1038h], xmm0 }
  *(_DWORD *)(_RBX + 4168) = 700;
  *(_DWORD *)(_RBX + 4172) = 0;
  *(_QWORD *)(_RBX + 4176) = 0;
  *(double *)&_XMM0 = sub_9290((int)_RBX + 4184, 32, (unsigned int)"Unknown Thread!", v4, v5, v6);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [rbx+1078h], xmm0 }
  *(_DWORD *)(_RBX + 4232) = 0;
  *(_QWORD *)(_RBX + 4272) = 0;
  __asm
  {
    vmovups xmmword ptr [rbx+109Ch], xmm0
    vmovups xmmword ptr [rbx+1090h], xmm0
  }
  *(_DWORD *)(_RBX + 4280) = 0;
  scePthreadMutexattrInit(v12);
  scePthreadMutexattrSettype(v12, 2);
  scePthreadMutexInit(_RBX + 4288, v12, "hx crit sec");
  scePthreadMutexattrDestroy(v12);
  *(_DWORD *)(_RBX + 4296) = 0;
  scePthreadMutexattrInit(v12);
  scePthreadMutexattrSettype(v12, 2);
  scePthreadMutexInit(_RBX + 4304, v12, "hx crit sec");
  *(double *)&_XMM0 = scePthreadMutexattrDestroy(v12);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [rbx+10D8h], xmm0 }
  *(_QWORD *)(_RBX + 4328) = 0;
  nullsub_18(_RBX + 4336, "EASTL list");
  *(_QWORD *)(_RBX + 4312) = _RBX + 4312;
  *(_QWORD *)(_RBX + 4320) = _RBX + 4312;
  *(_DWORD *)(_RBX + 4344) = -1;
  g_orbis_render_system = _RBX;
  return 0x6365786562696C2FLL;
}
