__int64 __fastcall fmod_bank_resource_resolve_platform_path(__int64 a1, _QWORD *a2)
{
  _BYTE *v4; // rax
  _BYTE *v5; // rbx
  _BYTE *v6; // r12
  unsigned __int64 v7; // rbx
  __int64 v8; // rdi
  __int64 v9; // rax
  __int64 result; // rax
  __int64 v11; // rax
  __int64 v12; // rax

  v4 = (_BYTE *)sub_1AE5B0(*(_QWORD *)(a1 + 8));
  v5 = (_BYTE *)a2[1];
  v6 = v4;
  if ( v5 != v4 )
  {
    if ( v4 != nullptr && *v4 != 0 )
    {
      v7 = strlen(v4);
      (*(void (__fastcall **)(_QWORD *, unsigned __int64))(*a2 + 24LL))(a2, v7);
      v8 = a2[1];
      if ( *(unsigned int *)(v8 - 4) < v7 )
        v7 = *(unsigned int *)(v8 - 4);
      memmove(v8, v6, v7);
      v5 = (_BYTE *)(a2[1] + v7);
    }
    *v5 = 0;
  }
  v9 = sub_254500(a2, "desktop");
  if ( v9 == -1 )
    v9 = sub_254500(a2, "Desktop");
  if ( v9 != -1 )
    sub_254C30(a2, v9, 7, "PS4");
  if ( (unsigned __int8)sub_2548C0(a2, "_eng.bank") != 0 || (_BYTE)(result = sub_2548C0(a2, "/eng.bank")) != 0 )
  {
    *(_BYTE *)(a1 + 112) = 1;
    v11 = strlen(a2[1]);
    sub_255AB0(a2, v11 - 8);
    v12 = sub_5FA0(&g_sound_manager);
    sub_258900(a2, v12);
    return sub_2588E0(a2, ".bank");
  }
  return result;
}
