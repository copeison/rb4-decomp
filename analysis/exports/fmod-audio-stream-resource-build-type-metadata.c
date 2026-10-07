__int64 __fastcall fmod_audio_stream_resource_build_type_metadata(__int64 *a1)
{
  char *v2; // rbx
  char *v3; // rsi
  __int64 v4; // r13
  __int64 v5; // rax
  __int64 v6; // r15
  char *v7; // rbx
  __int64 v8; // rbx
  char *v9; // rbx
  char *v10; // rsi
  __int64 v11; // r13
  __int64 v12; // rax
  __int64 v13; // r15
  char *v14; // rbx
  __int64 v15; // rbx
  char *v16; // rbx
  char *v17; // rsi
  __int64 v18; // r13
  __int64 v19; // rax
  __int64 v20; // r15
  char *v21; // rbx
  __int64 v22; // rbx
  char *v23; // rbx
  char *v24; // rsi
  __int64 v25; // r13
  __int64 v26; // rax
  __int64 v27; // r15
  char *v28; // rbx
  __int64 v29; // rbx
  char *v30; // rbx
  char *v31; // rsi
  __int64 v32; // r13
  __int64 v33; // rax
  __int64 v34; // r15
  char *v35; // rbx
  __int64 v36; // rbx
  __int64 v38; // [rsp+0h] [rbp-60h] BYREF
  __int64 v39; // [rsp+8h] [rbp-58h] BYREF
  __int64 v40; // [rsp+10h] [rbp-50h] BYREF
  __int64 v41; // [rsp+18h] [rbp-48h] BYREF
  __int64 v42; // [rsp+20h] [rbp-40h] BYREF
  _QWORD v43[7]; // [rsp+28h] [rbp-38h] BYREF

  v43[1] = 0x6365786562696C2FLL;
  sub_256FD0(v43, "mp3");
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
    *(_QWORD *)&v7[v6] = v43[0];
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
    *(_QWORD *)v2 = v43[0];
  }
  sub_256FD0(&v42, "wav");
  v9 = (char *)a1[1];
  if ( (unsigned __int64)v9 >= a1[2] )
  {
    v10 = (char *)*a1;
    v11 = 1;
    if ( v9 != (char *)*a1 )
      v11 = (__int64)&v9[-*a1] >> 2;
    if ( v11 != 0 )
    {
      v12 = sub_252CF0(a1 + 3, 8 * v11, 0);
      v10 = (char *)*a1;
      v9 = (char *)a1[1];
      v13 = v12;
    }
    else
    {
      v13 = 0;
    }
    v14 = (char *)(v9 - v10);
    memmove(v13, v10, v14);
    *(_QWORD *)&v14[v13] = v42;
    v15 = (__int64)&v14[v13 + 8];
    if ( *a1 != 0 )
      sub_252D30(a1 + 3, *a1, a1[2] - *a1);
    *a1 = v13;
    a1[1] = v15;
    a1[2] = v13 + 8 * v11;
  }
  else
  {
    a1[1] = (__int64)(v9 + 8);
    *(_QWORD *)v9 = v42;
  }
  sub_256FD0(&v41, "aac");
  v16 = (char *)a1[1];
  if ( (unsigned __int64)v16 >= a1[2] )
  {
    v17 = (char *)*a1;
    v18 = 1;
    if ( v16 != (char *)*a1 )
      v18 = (__int64)&v16[-*a1] >> 2;
    if ( v18 != 0 )
    {
      v19 = sub_252CF0(a1 + 3, 8 * v18, 0);
      v17 = (char *)*a1;
      v16 = (char *)a1[1];
      v20 = v19;
    }
    else
    {
      v20 = 0;
    }
    v21 = (char *)(v16 - v17);
    memmove(v20, v17, v21);
    *(_QWORD *)&v21[v20] = v41;
    v22 = (__int64)&v21[v20 + 8];
    if ( *a1 != 0 )
      sub_252D30(a1 + 3, *a1, a1[2] - *a1);
    *a1 = v20;
    a1[1] = v22;
    a1[2] = v20 + 8 * v18;
  }
  else
  {
    a1[1] = (__int64)(v16 + 8);
    *(_QWORD *)v16 = v41;
  }
  sub_256FD0(&v40, "ogg");
  v23 = (char *)a1[1];
  if ( (unsigned __int64)v23 >= a1[2] )
  {
    v24 = (char *)*a1;
    v25 = 1;
    if ( v23 != (char *)*a1 )
      v25 = (__int64)&v23[-*a1] >> 2;
    if ( v25 != 0 )
    {
      v26 = sub_252CF0(a1 + 3, 8 * v25, 0);
      v24 = (char *)*a1;
      v23 = (char *)a1[1];
      v27 = v26;
    }
    else
    {
      v27 = 0;
    }
    v28 = (char *)(v23 - v24);
    memmove(v27, v24, v28);
    *(_QWORD *)&v28[v27] = v40;
    v29 = (__int64)&v28[v27 + 8];
    if ( *a1 != 0 )
      sub_252D30(a1 + 3, *a1, a1[2] - *a1);
    *a1 = v27;
    a1[1] = v29;
    a1[2] = v27 + 8 * v25;
  }
  else
  {
    a1[1] = (__int64)(v23 + 8);
    *(_QWORD *)v23 = v40;
  }
  sub_256FD0(&v39, "m4a");
  v30 = (char *)a1[1];
  if ( (unsigned __int64)v30 >= a1[2] )
  {
    v31 = (char *)*a1;
    v32 = 1;
    if ( v30 != (char *)*a1 )
      v32 = (__int64)&v30[-*a1] >> 2;
    if ( v32 != 0 )
    {
      v33 = sub_252CF0(a1 + 3, 8 * v32, 0);
      v31 = (char *)*a1;
      v30 = (char *)a1[1];
      v34 = v33;
    }
    else
    {
      v34 = 0;
    }
    v35 = (char *)(v30 - v31);
    memmove(v34, v31, v35);
    *(_QWORD *)&v35[v34] = v39;
    v36 = (__int64)&v35[v34 + 8];
    if ( *a1 != 0 )
      sub_252D30(a1 + 3, *a1, a1[2] - *a1);
    *a1 = v34;
    a1[1] = v36;
    a1[2] = v34 + 8 * v32;
  }
  else
  {
    a1[1] = (__int64)(v30 + 8);
    *(_QWORD *)v30 = v39;
  }
  sub_256FD0(&v38, "Streaming Audio");
  a1[4] = v38;
  *((_BYTE *)a1 + 40) = 0;
  *((_BYTE *)a1 + 44) = 1;
  *((_BYTE *)a1 + 46) = 1;
  *((_DWORD *)a1 + 12) = 1;
  return 0x6365786562696C2FLL;
}
