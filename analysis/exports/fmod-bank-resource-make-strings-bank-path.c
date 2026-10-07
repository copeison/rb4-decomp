_QWORD *__fastcall fmod_bank_resource_make_strings_bank_path(_QWORD *a1, __int64 a2, __int64 a3)
{
  _BYTE *v5; // r15
  _BYTE *v6; // r14
  unsigned __int64 v7; // r14
  __int64 v8; // rdi
  __int64 v9; // rax

  sub_255080(a1);
  sub_2553B0(a1, 512);
  v5 = *(_BYTE **)(a3 + 8);
  v6 = (_BYTE *)a1[1];
  if ( v6 != v5 )
  {
    if ( v5 != nullptr && *v5 != 0 )
    {
      v7 = strlen(v5);
      (*(void (__fastcall **)(_QWORD *, unsigned __int64))(*a1 + 24LL))(a1, v7);
      v8 = a1[1];
      if ( *(unsigned int *)(v8 - 4) < v7 )
        v7 = *(unsigned int *)(v8 - 4);
      memmove(v8, v5, v7);
      v6 = (_BYTE *)(a1[1] + v7);
    }
    *v6 = 0;
  }
  v9 = sub_254670(a1, 46);
  sub_254FB0(a1, v9);
  sub_2542E0(a1, ".strings.bank");
  return a1;
}
