__int64 *__fastcall fmod_studio_sound_manager_create(__int64 a1, _QWORD *a2)
{
  __int64 v4; // rbx
  __int64 v5; // rax
  _QWORD *v6; // r15
  int v7; // eax
  __int64 v8; // rax
  char *v9; // rbx
  __int64 v11; // rbx
  int v12; // eax
  __int64 v13; // rcx
  __int64 *v14; // rax
  __int64 v15; // rcx
  __int64 *v16; // r15
  __int64 v17; // [rsp+8h] [rbp-168h]
  __int64 v18; // [rsp+10h] [rbp-160h]
  _BYTE v19[16]; // [rsp+18h] [rbp-158h] BYREF
  void *v20; // [rsp+28h] [rbp-148h] BYREF
  char *v21; // [rsp+30h] [rbp-140h]
  int v22; // [rsp+38h] [rbp-138h]
  char v23; // [rsp+3Ch] [rbp-134h] BYREF
  __int64 v24; // [rsp+140h] [rbp-30h]

  v24 = 0x6365786562696C2FLL;
  if ( *((_DWORD *)a2 + 24) == 4 )
    return nullptr;
  v5 = sub_C14B0(&unk_19C90B0, a2[8], 1);
  v6 = (_QWORD *)v5;
  if ( v5 == 0 )
    return nullptr;
  v7 = *(_DWORD *)(v5 + 16);
  if ( (unsigned int)(v7 - 1) > 1 )
    return nullptr;
  if ( v7 == 2 )
  {
    v8 = v6[95];
    if ( v8 != 0 )
    {
      if ( v6[96] == 0 )
        return nullptr;
      goto LABEL_12;
    }
    return nullptr;
  }
  v8 = v6[35];
  if ( v8 == 0 )
    return nullptr;
  if ( v6[36] == 0 )
    return nullptr;
LABEL_12:
  v18 = v8;
  v20 = &unk_18E6AC8;
  sub_258550(&v20);
  v21 = &v23;
  v22 = 256;
  v23 = 0;
  v20 = &unk_18E6AC8;
  v9 = (char *)*a2;
  if ( ((unsigned __int64)strlen(*a2) < 7 || v9[5] != 58) && ((unsigned __int64)strlen(v9) < 0xA || v9[8] != 58) )
  {
    sub_2542E0(&v20, "event:/");
    sub_2542E0(&v20, *a2);
    v9 = v21;
  }
  if ( (unsigned int)FMOD::Studio::System::lookupID(v18, v9, v19) != 0 )
    return nullptr;
  v17 = (__int64)v9;
  v11 = a2[1];
  if ( v11 == 0 )
    v11 = sub_5C20(&g_sound_manager);
  scePthreadMutexLock(a1 + 24);
  v12 = *(_DWORD *)(a1 + 16) + 1;
  *(_DWORD *)(a1 + 16) = v12;
  v13 = *(_QWORD *)(a1 + 48);
  if ( v13 != 0 )
  {
    v14 = *(__int64 **)(a1 + 32);
    v14[2] = 0;
    *(_QWORD *)(a1 + 48) = v13 - 1;
    v15 = *v14;
    *(_QWORD *)(v15 + 8) = v14[1];
    *(_QWORD *)v14[1] = v15;
    *v14 = (__int64)v14;
    v14[1] = (__int64)v14;
    v14[3] = v11;
    v14[4] = (__int64)v6;
    v16 = v14 - 5;
    sub_40720((__int64)(v14 - 5));
    v12 = *(_DWORD *)(a1 + 16);
  }
  else
  {
    v16 = nullptr;
  }
  *(_DWORD *)(a1 + 16) = v12 - 1;
  scePthreadMutexUnlock(a1 + 24);
  v4 = 0;
  if ( v16 != nullptr && fmod_studio_sound_generator_initialize_event(v16, v17, (__int64)a2, nullptr, nullptr) != 0 )
    return v16;
  return (__int64 *)v4;
}
