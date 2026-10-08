// Loads ../../system/data/render/default_lighting.scene and locates default directional/spot light objects. Descriptive inferred name.
char __fastcall render_load_default_lighting(_QWORD *a1)
{
  __int64 v2; // rcx
  __int64 v3; // rsi
  __int64 v4; // r15
  _QWORD *v5; // r12
  __int64 v6; // rdi
  __int64 v7; // rdi
  __int64 v8; // r15
  __int64 v9; // rax
  __int64 v10; // rdx
  __int64 v11; // rdi
  _QWORD *v12; // rax
  __int64 v14; // rax
  __int64 v15; // rax
  __int64 v16; // rcx
  __int64 v17; // rdx
  _QWORD *v18; // rdi
  __int64 v19; // rbx
  __int64 v20; // rsi
  __int64 v21; // rdx
  __int64 v22; // rax
  __int64 v24; // rsi
  _QWORD *v25; // rcx
  char *v26; // rbx
  int v27; // ecx
  __int64 v28; // r13
  char *v29; // rsi
  __int64 v30; // rax
  __int64 v31; // r12
  char *v32; // rbx
  __int64 v33; // rbx
  __int64 v34; // rsi
  __int64 v35; // rax
  __int64 v36; // rbx
  __int64 v37; // rdx
  _QWORD *v38; // rax
  int v39; // ecx
  char *v40; // rbx
  __int64 v41; // r13
  char *v42; // rsi
  __int64 v43; // rax
  __int64 v44; // r12
  char *v45; // rbx
  __int64 v46; // rbx
  __int64 v47; // rsi
  __int64 v48; // rax
  __int64 v49; // rsi
  __int64 v50; // rdx
  __int64 v51; // rcx
  _QWORD *v52; // rax
  __int64 v53; // rdi
  _QWORD *v58; // [rsp+8h] [rbp-68h]
  int v59; // [rsp+14h] [rbp-5Ch]
  int v60; // [rsp+14h] [rbp-5Ch]
  __int64 v61; // [rsp+18h] [rbp-58h] BYREF
  __int64 v62; // [rsp+20h] [rbp-50h] BYREF
  __int64 v63; // [rsp+28h] [rbp-48h] BYREF
  __int64 v64; // [rsp+30h] [rbp-40h] BYREF
  _QWORD v65[7]; // [rsp+38h] [rbp-38h] BYREF

  v65[1] = 0x6365786562696C2FLL;
  v64 = 19246190;
  sub_1AF950(&v64, "../../system/data/render/default_lighting.scene");
  render_load_scene_resource(v65, v64, 0);
  v4 = v65[0];
  v65[0] = 0;
  v58 = a1;
  v5 = a1 + 59;
  v6 = a1[59];
  if ( v6 != 0 )
  {
    sub_1ADEF0(v6);
    v7 = v65[0];
    *v5 = v4;
    if ( v7 != 0 )
    {
      sub_1ADEF0(v7);
      v4 = *v5;
      if ( *v5 == 0 )
        return 0;
      goto LABEL_7;
    }
  }
  else
  {
    *v5 = v4;
  }
  if ( v4 == 0 )
    return 0;
LABEL_7:
  v8 = *(_QWORD *)(v4 + 48);
  v9 = **(_QWORD **)(*(_QWORD *)(v8 + 176) + 8LL);
  v10 = *(unsigned int *)(v9 + 64);
  if ( *(_DWORD *)(v9 + 64) != 0 )
  {
    v11 = 0;
    v10 *= 24;
    v2 = unk_1A722A8;
    v12 = (_QWORD *)(*(_QWORD *)(v9 + 56) + 8LL);
    while ( *v12 != unk_1A722A8 )
    {
      v12 += 3;
      v10 -= 24;
      if ( v10 == 0 )
        goto LABEL_15;
    }
    v11 = *(v12 - 1);
  }
  else
  {
    v11 = 0;
  }
LABEL_15:
  v14 = sub_408F60(v11, v3, v10, v2);
  if ( v14 != 0 )
  {
    a1[60] = v14;
    a1[63] = a1[62];
    a1[67] = a1[66];
    v15 = sub_EF180(v8);
    if ( v15 != 0 )
    {
      while ( 1 )
      {
        v16 = *(_QWORD *)(v15 + 56);
        v17 = *(unsigned int *)(v15 + 64);
        if ( unk_1A873B8 == 19246190 )
          goto LABEL_25;
        if ( (_DWORD)v17 != 0 )
          break;
LABEL_32:
        v15 = sub_EF1A0(v8, v15, 19246190, v16);
        if ( v15 == 0 )
          goto LABEL_33;
      }
      v18 = (_QWORD *)(v16 + 16);
      v19 = 24 * v17;
      while ( *v18 != unk_1A873B8 )
      {
        v18 += 3;
        v19 -= 24;
        if ( v19 == 0 )
          goto LABEL_25;
      }
      v20 = *(v18 - 2);
      if ( v20 != 0 )
        *(_BYTE *)(v20 + 22) = 0;
LABEL_25:
      if ( (_DWORD)v17 != 0 )
      {
        v16 += 8;
        v21 = 24 * v17;
        while ( *(_QWORD *)v16 != unk_1A88DE8 )
        {
          v16 += 24;
          v21 -= 24;
          if ( v21 == 0 )
            goto LABEL_32;
        }
        v16 = *(_QWORD *)(v16 - 8);
        if ( v16 != 0 )
          *(_BYTE *)(v16 + 22) = 0;
      }
      goto LABEL_32;
    }
LABEL_33:
    sub_256FD0(&v63, "default_directional");
    v22 = sub_F0FF0(v8, v63, 0);
    _R14 = v58;
    if ( v22 != 0 && *(_DWORD *)(v22 + 64) != 0 )
    {
      v24 = 24LL * *(unsigned int *)(v22 + 64);
      v25 = (_QWORD *)(*(_QWORD *)(v22 + 56) + 8LL);
      while ( *v25 != unk_1A87638 )
      {
        v25 += 3;
        v24 -= 24;
        if ( v24 == 0 )
          goto LABEL_53;
      }
      if ( *(v25 - 1) != 0 )
      {
        v26 = (char *)v58[63];
        v27 = *(_DWORD *)(v22 + 88);
        if ( (unsigned __int64)v26 >= v58[64] )
        {
          v59 = *(_DWORD *)(v22 + 88);
          v28 = 1;
          v29 = (char *)v58[62];
          if ( v26 != v29 )
            v28 = (v26 - v29) >> 1;
          if ( v28 != 0 )
          {
            v30 = sub_252CF0(v58 + 65, 4 * v28, 0);
            v29 = (char *)v58[62];
            v26 = (char *)v58[63];
            v31 = v30;
          }
          else
          {
            v31 = 0;
          }
          v32 = (char *)(v26 - v29);
          memmove(v31, v29, v32);
          *(_DWORD *)&v32[v31] = v59;
          v33 = (__int64)&v32[v31 + 4];
          v34 = v58[62];
          if ( v34 != 0 )
            sub_252D30(v58 + 65, v34, v58[64] - v34);
          v58[62] = v31;
          v58[63] = v33;
          v58[64] = v31 + 4 * v28;
        }
        else
        {
          v58[63] = v26 + 4;
          *(_DWORD *)v26 = v27;
        }
      }
    }
LABEL_53:
    sub_256FD0(&v62, "default_spot_with_shadows");
    v35 = sub_F0FF0(v8, v62, 0);
    v36 = v35;
    if ( v35 != 0 && *(_DWORD *)(v35 + 64) != 0 )
    {
      v37 = 24LL * *(unsigned int *)(v35 + 64);
      v38 = (_QWORD *)(*(_QWORD *)(v35 + 56) + 8LL);
      while ( *v38 != unk_1A892F8 )
      {
        v38 += 3;
        v37 -= 24;
        if ( v37 == 0 )
          goto LABEL_70;
      }
      if ( *(v38 - 1) != 0 )
      {
        render_configure_default_shadowed_spot(v58, v36, v37, unk_1A892F8);
        v39 = *(_DWORD *)(v36 + 88);
        v40 = (char *)v58[67];
        if ( (unsigned __int64)v40 >= v58[68] )
        {
          v60 = v39;
          v41 = 1;
          v42 = (char *)v58[66];
          if ( v40 != v42 )
            v41 = (v40 - v42) >> 1;
          if ( v41 != 0 )
          {
            v43 = sub_252CF0(v58 + 69, 4 * v41, 0);
            v42 = (char *)v58[66];
            v40 = (char *)v58[67];
            v44 = v43;
          }
          else
          {
            v44 = 0;
          }
          v45 = (char *)(v40 - v42);
          memmove(v44, v42, v45);
          *(_DWORD *)&v45[v44] = v60;
          v46 = (__int64)&v45[v44 + 4];
          v47 = v58[66];
          if ( v47 != 0 )
            sub_252D30(v58 + 69, v47, v58[68] - v47);
          v58[66] = v44;
          v58[67] = v46;
          v58[68] = v44 + 4 * v41;
        }
        else
        {
          v58[67] = v40 + 4;
          *(_DWORD *)v40 = v39;
        }
      }
    }
LABEL_70:
    sub_256FD0(&v61, "default_probe");
    v48 = sub_F0FF0(v8, v61, 0);
    if ( v48 != 0 )
    {
      if ( *(_DWORD *)(v48 + 64) != 0 )
      {
        v50 = 24LL * *(unsigned int *)(v48 + 64);
        v51 = unk_1A88DE8;
        v52 = (_QWORD *)(*(_QWORD *)(v48 + 56) + 8LL);
        while ( *v52 != unk_1A88DE8 )
        {
          v52 += 3;
          v50 -= 24;
          if ( v50 == 0 )
            goto LABEL_75;
        }
        v53 = *(v52 - 1);
        v58[61] = v53;
        if ( v53 != 0 )
        {
          *(_BYTE *)(v53 + 22) = 1;
          __asm
          {
            vmovss  xmm0, dword ptr [r14+234h]
            vaddss  xmm0, xmm0, xmm0
          }
          sub_498440(v53, v49, v50, v51, *(double *)&_XMM0);
          __asm { vmovss  xmm0, dword ptr [r14+234h] }
          __asm { vmulss  xmm0, xmm0, cs:dword_12B6C3C }
          sub_498480(v58[61], *(double *)&_XMM0);
        }
      }
      else
      {
LABEL_75:
        v58[61] = 0;
      }
    }
    render_apply_default_lighting_mode(v58);
    return 1;
  }
  else
  {
    if ( *v5 != 0 )
      sub_1ADEF0(*v5);
    *v5 = 0;
    return 0;
  }
}
