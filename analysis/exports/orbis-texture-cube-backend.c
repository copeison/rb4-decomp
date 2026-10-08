// orbis_texture_cube_destruct at 0x8E6BE0
__int64 __fastcall orbis_texture_cube_destruct(__int64 a1)
{
  __int64 v2; // rax
  __int64 v3; // r14
  unsigned int base_address_256; // eax
  __int64 v5; // rdi
  double v6; // xmm0_8
  __int64 v7; // rdi
  __int64 v8; // rdi

  *(_QWORD *)a1 = &unk_195FD08;
  v2 = *(_QWORD *)(a1 + 816);
  if ( v2 != 0 )
  {
    orbis_defer_allocation_release(g_orbis_render_system, (unsigned __int64)*(unsigned int *)(v2 + 28) << 8);
    v3 = g_orbis_render_system;
    base_address_256 = gnm_render_target_get_base_address_256(*(unsigned int **)(a1 + 816));
    orbis_defer_allocation_release(v3, (unsigned __int64)base_address_256 << 8);
    v5 = *(_QWORD *)(a1 + 816);
    if ( v5 != 0 )
      sub_37BF50(v5);
    *(_QWORD *)(a1 + 816) = 0;
  }
  orbis_defer_allocation_release(g_orbis_render_system, *(_QWORD *)(a1 + 800));
  v6 = orbis_defer_allocation_release(g_orbis_render_system, *(_QWORD *)(a1 + 808));
  v7 = *(_QWORD *)(a1 + 792);
  if ( v7 != 0 )
    v6 = sub_37BF50(v7);
  *(_QWORD *)(a1 + 792) = 0;
  v8 = *(_QWORD *)(a1 + 824);
  if ( v8 != 0 )
    v6 = sub_37BF50(v8);
  *(_QWORD *)(a1 + 824) = 0;
  return sub_6A0EA0(a1, v6);
}


// orbis_texture_cube_delete at 0x8E6CC0
double __fastcall orbis_texture_cube_delete(__int64 a1)
{
  __int64 v1; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  orbis_texture_cube_destruct(a1);
  return sub_37BF50(a1);
}


// orbis_texture_cube_initialize_backend at 0x8E6CE0
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_texture_cube_initialize_backend(__int64 a1)
{
  __int64 v2; // rbx
  unsigned int v3; // r12d
  unsigned int v4; // r15d
  unsigned int v5; // r14d
  int v6; // r12d
  unsigned int v7; // eax
  unsigned __int64 v10; // rax
  unsigned int v11; // r13d
  unsigned __int64 v12; // r12
  unsigned __int64 v13; // rax
  unsigned int v14; // r15d
  unsigned __int64 v15; // r14
  int v16; // eax
  __int64 v17; // rdi
  __int64 v18; // rsi
  __int64 v21; // r13
  unsigned int v22; // r14d
  unsigned int v23; // eax
  unsigned __int64 v26; // rax
  unsigned int v27; // r14d
  unsigned __int64 v28; // r15
  int v29; // eax
  __int64 v30; // rdi
  __int64 v31; // rsi
  __int64 v32; // rbx
  unsigned __int64 v33; // r13
  __int64 v34; // r14
  __int64 v35; // rdi
  char *v36; // rsi
  __int64 v39; // rsi
  unsigned int v40; // edx
  __int64 v41; // rcx
  __int64 v43; // [rsp+8h] [rbp-B8h]
  __int64 v44; // [rsp+10h] [rbp-B0h]
  __int64 v45; // [rsp+18h] [rbp-A8h] BYREF
  __int64 v46; // [rsp+20h] [rbp-A0h] BYREF
  _DWORD v47[12]; // [rsp+28h] [rbp-98h] BYREF
  _DWORD v48[13]; // [rsp+58h] [rbp-68h] BYREF
  int v49; // [rsp+8Ch] [rbp-34h] BYREF
  __int64 v50; // [rsp+90h] [rbp-30h]

  v2 = a1;
  v50 = 0x6365786562696C2FLL;
  v3 = *(_DWORD *)(a1 + 332);
  v4 = sub_10DEF20();
  if ( *(_DWORD *)(v2 + 64) == 2 )
  {
    v5 = sub_8E17C0(v3);
    v6 = sub_8E17F0(v3);
    v7 = sub_10C2C20(v5);
    sub_10F4A00(v4, v47, (unsigned int)(v6 == 0) + 3, v7, 1);
    sub_10C3220(v48);
    v48[0] = *(_DWORD *)(v2 + 112);
    v48[1] = *(_DWORD *)(v2 + 116);
    v48[2] = 0;
    v48[3] = 6;
    v48[4] = v5;
    v48[5] = v6;
    v48[6] = v47[0];
    v48[8] = 0;
    v48[7] = v4;
    _RAX = sub_37BF40(52);
    __asm { vxorps  ymm0, ymm0, ymm0 }
    __asm
    {
      vmovups ymmword ptr [rax+14h], ymm0
      vmovups ymmword ptr [rax], ymm0
    }
    *(_QWORD *)(v2 + 824) = _RAX;
    sub_10C3270(_RAX, v48);
    v10 = sub_10C3A30(*(_QWORD *)(v2 + 824));
    v11 = v10;
    v12 = HIDWORD(v10);
    v13 = sub_10C3AE0(*(_QWORD *)(v2 + 824));
    v14 = v13;
    v15 = HIDWORD(v13);
    if ( byte_1ADF2B0 == 0 )
    {
      _cxa_guard_acquire(&byte_1ADF2B0);
      if ( v16 != 0 )
      {
        qword_1ADF2A8 = sub_37BA70("gpu");
        _cxa_guard_release(&byte_1ADF2B0);
      }
    }
    sub_37A920(qword_1ADF2A8);
    *(_QWORD *)(v2 + 800) = sub_37AE70(v11, *(_QWORD *)(v2 + 152), (unsigned int)v12);
    *(_QWORD *)(v2 + 808) = sub_37AE70(v14, *(_QWORD *)(v2 + 152), (unsigned int)v15);
    sub_37A9B0(v17, v18);
    _RAX = sub_37BF40(32);
    __asm { vxorps  ymm0, ymm0, ymm0 }
    __asm { vmovups ymmword ptr [rax], ymm0 }
    *(_QWORD *)(v2 + 792) = _RAX;
    sub_10DE1C0(_RAX, *(_QWORD *)(v2 + 824), 1);
LABEL_20:
    v35 = *(_QWORD *)(v2 + 792);
    v36 = (_BYTE *)(&loc_6C + 1);
    goto LABEL_21;
  }
  v21 = v2 + 312;
  v22 = sub_8E1820(v2 + 64);
  v23 = sub_8E1790(v3);
  sub_10F4A00(v4, &v49, v22, v23, 1);
  sub_10DC780(v48);
  v48[0] = 11;
  v48[1] = *(_DWORD *)(v2 + 112);
  v48[2] = *(_DWORD *)(v2 + 116);
  v48[3] = 1;
  v48[4] = 0;
  v48[6] = 1;
  v48[7] = sub_8E1790(v3);
  v48[8] = v49;
  v48[10] = 0;
  v48[5] = sub_6832D0(v2 + 312) + 1;
  v48[9] = v4;
  _RAX = sub_37BF40(32);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [rax], ymm0 }
  *(_QWORD *)(v2 + 792) = _RAX;
  sub_10DC7A0(_RAX, v48);
  v26 = sub_10DCD20(*(_QWORD *)(v2 + 792));
  v27 = v26;
  v28 = HIDWORD(v26);
  if ( byte_1ADF2C0 == 0 )
  {
    _cxa_guard_acquire(&byte_1ADF2C0);
    if ( v29 != 0 )
    {
      qword_1ADF2B8 = sub_37BA70("gpu");
      _cxa_guard_release(&byte_1ADF2C0);
    }
  }
  sub_37A920(qword_1ADF2B8);
  *(_QWORD *)(v2 + 800) = sub_37AE70(v27, *(_QWORD *)(v2 + 152), (unsigned int)v28);
  sub_37A9B0(v30, v31);
  v44 = v2;
  if ( *(_QWORD *)(v2 + 336) != 0 )
  {
    v32 = 0;
    v43 = v21;
    do
    {
      if ( sub_6832D0(v21) != -1 )
      {
        v33 = 0;
        v34 = v44 + 80 * v32 + 312;
        do
        {
          sub_10FC210(v47, *(_QWORD *)(v44 + 792), (unsigned int)v33, (unsigned int)v32);
          sub_10FD200(&v46, &v45, *(_QWORD *)(v44 + 792), (unsigned int)v33, (unsigned int)v32);
          sub_10FDB10(v46 + *(_QWORD *)(v44 + 800), *(_QWORD *)(v34 + 24), v47);
          v34 = *(_QWORD *)(v34 + 40);
          ++v33;
        }
        while ( v33 < sub_6832D0(v43) + 1 );
      }
      v21 = v43;
      ++v32;
    }
    while ( v32 != 6 );
  }
  v2 = v44;
  sub_10DDED0(*(_QWORD *)(v44 + 792), *(_QWORD *)(v44 + 800) >> 8);
  if ( (*(_BYTE *)(v44 + 104) & 2) != 0 )
  {
    _RAX = sub_37BF40(64);
    __asm { vxorps  ymm0, ymm0, ymm0 }
    __asm
    {
      vmovups ymmword ptr [rax+20h], ymm0
      vmovups ymmword ptr [rax], ymm0
    }
    *(_QWORD *)(v44 + 816) = _RAX;
    v39 = *(_QWORD *)(v44 + 792);
    v40 = *(_DWORD *)(v39 + 12);
    v41 = HIWORD(v40) & 0xF;
    if ( v40 <= 0xDFFFFFFF )
      v41 = 0;
    sub_10DAA70(_RAX, v39, 0, v41, 0, 0);
    goto LABEL_20;
  }
  v35 = *(_QWORD *)(v44 + 792);
  v36 = "so.1";
LABEL_21:
  sub_10DEA40(v35, v36);
  return 0x6365786562696C2FLL;
}


// orbis_texture_cube_render_target at 0x8E7270
__int64 __fastcall orbis_texture_cube_render_target(__int64 a1)
{
  return *(_QWORD *)(a1 + 816);
}


// orbis_texture_cube_depth_target at 0x8E7280
__int64 __fastcall orbis_texture_cube_depth_target(__int64 a1)
{
  return *(_QWORD *)(a1 + 824);
}

