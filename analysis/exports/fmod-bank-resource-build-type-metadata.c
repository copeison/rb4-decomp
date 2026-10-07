__int64 __fastcall fmod_bank_resource_build_type_metadata(__int64 *a1)
{
  char *v2; // rbx
  char *v3; // rsi
  __int64 v4; // r13
  __int64 v5; // rax
  __int64 v6; // r15
  char *v7; // rbx
  __int64 v8; // rbx
  __int64 v10; // [rsp+0h] [rbp-40h] BYREF
  _QWORD v11[7]; // [rsp+8h] [rbp-38h] BYREF

  v11[1] = 0x6365786562696C2FLL;
  sub_256FD0(v11, "bank");
  v2 = (char *)a1[1];
  if ( (unsigned __int64)v2 >= a1[2] )
  {
    v3 = (char *)*a1;
    v4 = 1;
    if ( v2 != (char *)*a1 )
      v4 = (__int64)&v2[-*a1] >> 2;
    if ( v4 != 0 )
    {
      v5 = sub_252CF0(a1 + 3, 8 * v4, 0);
      v3 = (char *)*a1;
      v2 = (char *)a1[1];
      v6 = v5;
    }
    else
    {
      v6 = 0;
    }
    v7 = (char *)(v2 - v3);
    memmove(v6, v3, v7);
    *(_QWORD *)&v7[v6] = v11[0];
    v8 = (__int64)&v7[v6 + 8];
    if ( *a1 != 0 )
      sub_252D30(a1 + 3, *a1, a1[2] - *a1);
    *a1 = v6;
    a1[1] = v8;
    a1[2] = v6 + 8 * v4;
  }
  else
  {
    a1[1] = (__int64)(v2 + 8);
    *(_QWORD *)v2 = v11[0];
  }
  sub_256FD0(&v10, "FMod Banks");
  a1[4] = v10;
  *((_BYTE *)a1 + 46) = 1;
  *((_DWORD *)a1 + 12) = 1;
  return 0x6365786562696C2FLL;
}
