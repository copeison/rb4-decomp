__int64 __fastcall render_initialize_default_resources(__int64 *a1, __int64 a2)
{
  _QWORD *v3; // rbx
  __int64 v4; // r14
  __int64 v6; // rdx
  __int64 v7; // rcx
  __int64 v8; // rsi
  __int64 v9; // r8
  __int64 v10; // r9
  __int64 v12; // rdx
  __int64 v13; // rcx
  __int64 v14; // rsi
  __int64 v15; // r8
  __int64 v16; // r9
  _BYTE *v17; // rbx
  __int64 v18; // rdx
  __int64 v19; // rcx
  __int64 v20; // rdi
  __int128 v22; // [rsp+0h] [rbp-70h] BYREF
  int *v23; // [rsp+10h] [rbp-60h]
  int v24; // [rsp+24h] [rbp-4Ch]
  const char *v25; // [rsp+28h] [rbp-48h]
  int v26; // [rsp+3Ch] [rbp-34h] BYREF
  __int64 v27; // [rsp+40h] [rbp-30h]

  v27 = 0x6365786562696C2FLL;
  if ( (*(_BYTE *)(a2 + 1) | unk_19E4559) != 0 )
  {
    v3 = (_QWORD *)sub_37BF40(784);
    rnd_scene_resource_construct(v3);
    sub_1ADEB0(v3);
    if ( *a1 != 0 )
      sub_1ADEF0(*a1);
    *a1 = (__int64)v3;
    v4 = (*(__int64 (__fastcall **)(_QWORD *))(*v3 + 104LL))(v3);
    render_create_default_textures((__int64)a1);
    v26 = 0;
    render_compute_buffer_descriptor_init(&v22);
    __asm { vmovups xmm0, cs:xmmword_12B6B30 }
    v25 = "Default Compute Buffer";
    __asm { vmovups [rbp+var_70], xmm0 }
    v23 = &v26;
    v24 = 0;
    a1[50] = ((__int64 (__fastcall *)(__int128 *, __int64, __int64, __int64, __int64, __int64))render_create_compute_buffer)(
               &v22,
               v8,
               v6,
               v7,
               v9,
               v10);
    v26 = 1;
    render_compute_buffer_descriptor_init(&v22);
    __asm { vmovups xmm0, cs:xmmword_12B6B30 }
    v25 = "Default Compute Buffer";
    __asm { vmovups [rbp+var_70], xmm0 }
    v23 = &v26;
    v24 = 0;
    a1[51] = ((__int64 (__fastcall *)(__int128 *, __int64, __int64, __int64, __int64, __int64))render_create_compute_buffer)(
               &v22,
               v14,
               v12,
               v13,
               v15,
               v16);
    v17 = (_BYTE *)sub_F0A10(v4, 0, 0);
    sub_256FD0(&v22, "default_cam");
    sub_1160F0(v17, v22);
    a1[52] = sub_117700(v17, unk_1AAF8F8, 0);
    render_create_default_materials(a1, v4);
    if ( render_load_default_lighting(a1) == 0 )
      render_create_fallback_default_lighting(a1, v4, v18, v19);
    sub_FD990(*a1);
    sub_FD850(*a1, *(_QWORD *)(*a1 + 48), 1);
    v20 = a1[59];
    if ( v20 != 0 )
      sub_FD850(v20, *(_QWORD *)(v20 + 48), 1);
  }
  return 0x6365786562696C2FLL;
}
