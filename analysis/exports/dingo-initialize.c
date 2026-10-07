// Initializes the Dingo backend configuration, server list, regional client ID, required sysmodule, and platform networking state.
void __fastcall dingo_initialize(void *service)
{
  __int64 v1; // rax
  _QWORD *v2; // rax
  _QWORD *v3; // r14
  __int64 v4; // rax
  _QWORD *v5; // rax
  __int64 v6; // rax
  __int64 v7; // r15
  char v8; // bl
  __int64 v9; // rax
  __int64 *v10; // rsi
  char *v11; // rdx
  __int64 v12; // rsi
  _QWORD *v13; // rax
  __int64 v14; // rcx
  _QWORD **v15; // rsi
  unsigned int v16; // r15d
  char *v17; // r14
  _QWORD *v18; // r13
  __int64 v19; // r12
  int v20; // ebx
  __int64 v21; // rax
  _QWORD *v22; // rdx
  _QWORD *i; // rax
  char v24; // [rsp+0h] [rbp-D0h] BYREF
  char v25; // [rsp+8h] [rbp-C8h] BYREF
  __int64 v26; // [rsp+10h] [rbp-C0h]
  _QWORD **v27; // [rsp+18h] [rbp-B8h]
  __int64 v28; // [rsp+20h] [rbp-B0h]
  char *v29; // [rsp+28h] [rbp-A8h]
  _QWORD *v30; // [rsp+30h] [rbp-A0h]
  int v31; // [rsp+3Ch] [rbp-94h] BYREF
  __int64 v32; // [rsp+40h] [rbp-90h] BYREF
  __int64 v33; // [rsp+48h] [rbp-88h] BYREF
  __int64 v34; // [rsp+50h] [rbp-80h] BYREF
  __int64 v35; // [rsp+58h] [rbp-78h] BYREF
  __int64 v36; // [rsp+60h] [rbp-70h] BYREF
  __int64 v37; // [rsp+68h] [rbp-68h] BYREF
  __int64 v38; // [rsp+70h] [rbp-60h] BYREF
  __int64 v39; // [rsp+78h] [rbp-58h] BYREF
  __int64 v40; // [rsp+80h] [rbp-50h] BYREF
  __int64 v41; // [rsp+88h] [rbp-48h] BYREF
  __int64 v42; // [rsp+90h] [rbp-40h] BYREF
  _QWORD v43[7]; // [rsp+98h] [rbp-38h] BYREF

  v30 = service;
  v43[1] = 0x6365786562696C2FLL;
  if ( byte_19FBC80 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBC80) != 0 )
  {
    qword_19FBC78 = 19246190;
    _cxa_guard_release(&byte_19FBC80);
  }
  if ( qword_19FBC78 == 19246190 )
  {
    sub_256FD0(v43, "net");
    qword_19FBC78 = v43[0];
  }
  if ( byte_19FBC90 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBC90) != 0 )
  {
    qword_19FBC88 = 19246190;
    _cxa_guard_release(&byte_19FBC90);
  }
  if ( qword_19FBC88 == 19246190 )
  {
    sub_256FD0(&v42, "dingo");
    qword_19FBC88 = v42;
  }
  if ( byte_19FBCA0 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBCA0) != 0 )
  {
    qword_19FBC98 = 19246190;
    _cxa_guard_release(&byte_19FBCA0);
  }
  if ( qword_19FBC98 == 19246190 )
  {
    sub_256FD0(&v41, "servers");
    qword_19FBC98 = v41;
  }
  if ( byte_19FBCB0 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBCB0) != 0 )
  {
    qword_19FBCA8 = 19246190;
    _cxa_guard_release(&byte_19FBCB0);
  }
  if ( qword_19FBCA8 == 19246190 )
  {
    sub_256FD0(&v40, "hostname");
    qword_19FBCA8 = v40;
  }
  if ( byte_19FBCC0 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBCC0) != 0 )
  {
    qword_19FBCB8 = 19246190;
    _cxa_guard_release(&byte_19FBCC0);
  }
  if ( qword_19FBCB8 == 19246190 )
  {
    sub_256FD0(&v39, "port");
    qword_19FBCB8 = v39;
  }
  if ( byte_19FBCD0 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBCD0) != 0 )
  {
    qword_19FBCC8 = 19246190;
    _cxa_guard_release(&byte_19FBCD0);
  }
  if ( qword_19FBCC8 == 19246190 )
  {
    sub_256FD0(&v38, "use_ssl");
    qword_19FBCC8 = v38;
  }
  if ( byte_19FBCE0 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBCE0) != 0 )
  {
    qword_19FBCD8 = 19246190;
    _cxa_guard_release(&byte_19FBCE0);
  }
  if ( qword_19FBCD8 == 19246190 )
  {
    sub_256FD0(&v37, "client_id_scea");
    qword_19FBCD8 = v37;
  }
  if ( byte_19FBCF0 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBCF0) != 0 )
  {
    qword_19FBCE8 = 19246190;
    _cxa_guard_release(&byte_19FBCF0);
  }
  if ( qword_19FBCE8 == 19246190 )
  {
    sub_256FD0(&v36, "client_id_scee");
    qword_19FBCE8 = v36;
  }
  if ( byte_19FBD00 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBD00) != 0 )
  {
    qword_19FBCF8 = 19246190;
    _cxa_guard_release(&byte_19FBD00);
  }
  if ( qword_19FBCF8 == 19246190 )
  {
    sub_256FD0(&v35, "default_environment");
    qword_19FBCF8 = v35;
  }
  if ( byte_19FBD10 == 0 && (unsigned int)_cxa_guard_acquire(&byte_19FBD10) != 0 )
  {
    qword_19FBD08 = 19246190;
    _cxa_guard_release(&byte_19FBD10);
  }
  if ( qword_19FBD08 == 19246190 )
  {
    sub_256FD0(&v34, "default_server");
    qword_19FBD08 = v34;
  }
  v1 = sub_368B00(qword_19FBC78);
  v2 = (_QWORD *)sub_21C970(v1, qword_19FBD08, 0);
  v3 = v30;
  v30[40] = sub_E850(*v2 + 16LL, v2);
  v4 = sub_368B00(qword_19FBC78);
  v5 = (_QWORD *)sub_21C970(v4, qword_19FBCF8, 0);
  v6 = sub_E850(*v5 + 16LL, v5);
  v7 = sub_368CD0(qword_19FBC78, v6);
  v8 = sub_36F600(unk_19FE988);
  v9 = sub_368B00(qword_19FBC78);
  v10 = &qword_19FBCE8;
  v11 = &v25;
  if ( v8 != 0 )
  {
    v10 = &qword_19FBCD8;
    v11 = &v24;
  }
  v12 = *v10;
  *(_QWORD *)v11 = v12;
  v13 = (_QWORD *)sub_21C970(v9, v12, 0);
  v3[41] = sub_237570(*v13 + 16LL, v13);
  v14 = sub_21C970(v7, qword_19FBC98, 0);
  v15 = (_QWORD **)(v3 + 36);
  if ( *(unsigned __int16 *)(v14 + 20) >= 2u )
  {
    v16 = 1;
    v17 = "so.1";
    v27 = v15;
    v28 = v14;
    v29 = (char *)(v30 + 39);
    do
    {
      sub_5F0B0(&v33, &v17[*(_QWORD *)v14], v14, v14);
      v18 = (_QWORD *)v33;
      if ( v33 != 0 && _InterlockedExchangeAdd((volatile signed __int32 *)(v33 + 16), 0xFFFFFFFF) == 1 )
      {
        sub_21E500(v18);
        sub_385460(24, v18);
      }
      v26 = sub_E850(*v18, v18);
      v32 = 0;
      sub_21CF20(v18, qword_19FBCA8, &v32, 1);
      v19 = v32;
      v31 = 0;
      sub_21D060(v18, qword_19FBCB8, &v31, 1);
      v20 = v31;
      v21 = sub_252CF0(v29, 40, 0);
      ++v16;
      v17 += 16;
      *(_QWORD *)(v21 + 16) = v26;
      *(_QWORD *)(v21 + 24) = v19;
      *(_DWORD *)(v21 + 32) = v20;
      v15 = v27;
      *(_QWORD *)v21 = v27;
      v22 = v30;
      *(_QWORD *)(v21 + 8) = v30[37];
      *(_QWORD *)v22[37] = v21;
      v22[37] = v21;
      ++v22[38];
      v14 = v28;
    }
    while ( v16 < *(__int16 *)(v28 + 20) );
  }
  for ( i = *v15; i != v15; i = (_QWORD *)*i )
    ;
  sceSysmoduleLoadModule(157);
  sub_33AEF0(v30);
}
