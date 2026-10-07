__int64 *__fastcall fmod_dialog_manager_create(__int64 a1, _QWORD *a2)
{
  __int64 v2; // r15
  __int64 v5; // rax
  int v6; // edx
  __int64 *result; // rax
  char *v8; // r13
  __int64 v9; // r15
  __int64 v10; // rbx
  int v11; // eax
  __int64 v12; // rcx
  __int64 *v13; // rax
  __int64 v14; // rcx
  __int64 *v15; // r15
  double v16; // xmm0_8
  char v17; // cl
  __int64 v18; // [rsp+8h] [rbp-168h]
  char v19[16]; // [rsp+18h] [rbp-158h] BYREF
  void *v20; // [rsp+28h] [rbp-148h] BYREF
  char *v21; // [rsp+30h] [rbp-140h]
  int v22; // [rsp+38h] [rbp-138h]
  char v23; // [rsp+3Ch] [rbp-134h] BYREF
  __int64 v24; // [rsp+140h] [rbp-30h]

  v24 = 0x6365786562696C2FLL;
  if ( *((_DWORD *)a2 + 24) != 4 )
    return nullptr;
  v5 = sub_C14B0(&unk_19C90B0, a2[8], 1);
  v6 = *(_DWORD *)(v5 + 16);
  if ( (unsigned int)(v6 - 1) <= 1 )
  {
    if ( v6 == 2 )
      v2 = *(_QWORD *)(v5 + 760);
    else
      v2 = *(_QWORD *)(v5 + 280);
  }
  v18 = v5;
  v20 = &unk_18E6AC8;
  sub_258550(&v20);
  v21 = &v23;
  v22 = 256;
  v23 = 0;
  v20 = &unk_18E6AC8;
  v8 = (char *)*a2;
  if ( (unsigned __int64)strlen(*a2) < 6 || v8[5] != 58 )
  {
    sub_2542E0(&v20, "event:/");
    sub_2542E0(&v20, *a2);
    v8 = v21;
  }
  if ( (unsigned int)FMOD::Studio::System::lookupID(v2, v8, v19) != 0 )
    return nullptr;
  v9 = a2[1];
  if ( v9 == 0 )
    v9 = sub_5C20(&g_sound_manager);
  v10 = unk_19C90E8;
  scePthreadMutexLock(a1 + 24);
  v11 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v11;
  v12 = *(_QWORD *)(a1 + 48);
  if ( v12 != 0 )
  {
    if ( v18 != 0 )
      v10 = v18;
    v13 = *(__int64 **)(a1 + 32);
    v13[2] = 0;
    *(_QWORD *)(a1 + 48) = v12 - 1;
    v14 = *v13;
    *(_QWORD *)(v14 + 8) = v13[1];
    *(_QWORD *)v13[1] = v14;
    *v13 = (__int64)v13;
    v13[1] = (__int64)v13;
    v13[3] = v9;
    v15 = v13 - 5;
    v13[4] = v10;
    sub_40720((__int64)(v13 - 5));
    v11 = *(_DWORD *)(a1 + 16);
  }
  else
  {
    v15 = nullptr;
  }
  *(_DWORD *)(a1 + 16) = v11 - 1;
  v16 = scePthreadMutexUnlock(a1 + 24);
  if ( v15 == nullptr )
    return nullptr;
  v17 = (*(__int64 (__fastcall **)(__int64 *, char *, _QWORD *, double))(*v15 + 256))(v15, v8, a2, v16);
  result = nullptr;
  if ( v17 != 0 )
    return v15;
  return result;
}
