__int64 *__fastcall fmod_buffered_stream_manager_create_from_options(__int64 a1, __int64 *a2, __int64 a3)
{
  __int64 v5; // rax
  __int64 v6; // rax
  __int64 v7; // r15
  __int64 v8; // r14
  int v9; // eax
  __int64 v10; // rcx
  __int64 *v11; // rax
  __int64 v12; // rcx
  __int64 v13; // r15
  __int64 v14; // rbx
  char v15; // r14
  __int64 *result; // rax
  __int64 v18; // [rsp+8h] [rbp-48h]
  __int64 v19; // [rsp+10h] [rbp-40h]
  _QWORD v20[7]; // [rsp+18h] [rbp-38h] BYREF

  v20[1] = 0x6365786562696C2FLL;
  v5 = *(_QWORD *)(a3 + 8);
  if ( v5 == 0 )
    v5 = sub_5C20(&g_sound_manager);
  v18 = v5;
  v19 = a3;
  v6 = sub_C14B0(&unk_19C90B0, *(_QWORD *)(a3 + 64), 1);
  v7 = unk_19C90E8;
  v8 = v6;
  scePthreadMutexLock(a1 + 24);
  v9 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v9;
  v10 = *(_QWORD *)(a1 + 48);
  if ( v10 != 0 )
  {
    v11 = *(__int64 **)(a1 + 32);
    if ( v8 != 0 )
      v7 = v8;
    v11[2] = 0;
    *(_QWORD *)(a1 + 48) = v10 - 1;
    v12 = *v11;
    *(_QWORD *)(v12 + 8) = v11[1];
    *(_QWORD *)v11[1] = v12;
    *v11 = (__int64)v11;
    v11[1] = (__int64)v11;
    v11[3] = v18;
    v11[4] = v7;
    v13 = (__int64)(v11 - 5);
    sub_40720((__int64)(v11 - 5));
    v9 = *(_DWORD *)(a1 + 16);
  }
  else
  {
    v13 = 0;
  }
  *(_DWORD *)(a1 + 16) = v9 - 1;
  scePthreadMutexUnlock(a1 + 24);
  if ( v13 == 0 )
    return nullptr;
  v14 = *a2;
  v20[0] = *a2;
  if ( v20[0] != 0 )
  {
    sub_1ADEB0(v14);
    v15 = fmod_buffered_stream_generator_initialize(v13, v20, v19);
    sub_1ADEF0(v14);
  }
  else
  {
    v15 = fmod_buffered_stream_generator_initialize(v13, v20, v19);
  }
  result = nullptr;
  if ( v15 != 0 )
    return (__int64 *)v13;
  return result;
}
