// Initializes one renderer platform configuration, including its sorted resolution list.
__int64 __fastcall render_platform_config_initialize(__int64 a1, unsigned int a2)
{
  __int64 v5; // rcx
  __int64 v7; // rax
  unsigned __int64 v9; // rcx
  __int64 v10; // rax
  unsigned __int64 v11; // rdx
  __int64 v13; // rbx
  double v14; // xmm0_8
  __int64 v15; // r14
  _QWORD *v16; // r12
  int v17; // eax
  unsigned __int64 v18; // r15
  __int64 v19; // rbx
  char *v20; // rbx
  char *v21; // r12
  unsigned __int64 v22; // r15
  __int64 v23; // rax
  _QWORD *v24; // rbx
  __int64 v25; // rsi
  __int64 v26; // r14
  __int64 v27; // rax
  char *v28; // rbx
  __int64 v29; // rdx
  __int64 *v30; // rcx
  __int64 v31; // r14
  __int64 *v32; // r14
  __int64 v33; // rbx
  __int64 v34; // r12
  _QWORD *v35; // rbx
  int v36; // eax
  __int64 i; // rcx
  __int64 v38; // r9
  __int64 v39; // r8
  __int64 v40; // rdx
  _QWORD *v41; // rsi
  __int64 v42; // rdi
  _QWORD *v43; // rbx
  __int64 v44; // rax
  _QWORD *v45; // rdx
  __int64 v46; // rcx
  bool v47; // cc
  _QWORD *v48; // r9
  __int64 v49; // r8
  __int64 v50; // rdx
  _QWORD *v51; // rsi
  __int64 v52; // rdi
  __int64 v54; // [rsp+8h] [rbp-78h]
  __int64 v55; // [rsp+18h] [rbp-68h]
  _QWORD *v56; // [rsp+20h] [rbp-60h]
  __int64 v57; // [rsp+28h] [rbp-58h]
  __int64 v58; // [rsp+38h] [rbp-48h] BYREF
  __int64 v59; // [rsp+40h] [rbp-40h] BYREF
  _QWORD v60[7]; // [rsp+48h] [rbp-38h] BYREF

  _R13 = (__int64 *)a1;
  v60[1] = 0x6365786562696C2FLL;
  switch ( a2 )
  {
    case 3u:
    case 5u:
    case 7u:
      __asm { vmovups ymm0, cs:ymmword_12B6580; jumptable 00000000006B99F8 cases 3,5,7 }
      v5 = 0x1601FFFFFF7FCC3LL;
      *(_QWORD *)(a1 + 32) = 8;
      *(_DWORD *)(a1 + 40) = 31;
      if ( a2 == 3 )
        v5 = 0x1A01FFFFFF7FCC3LL;
      __asm { vmovups ymmword ptr [r13+30h], ymm0 }
      *(_QWORD *)(a1 + 80) = 16;
      *(_BYTE *)(a1 + 88) = 1;
      *(_QWORD *)(a1 + 112) = v5;
      break;
    case 8u:
      __asm { vmovups ymm0, cs:ymmword_12B6580; jumptable 00000000006B99F8 case 8 }
      v9 = 57;
      *(_QWORD *)(a1 + 32) = 8;
      *(_DWORD *)(a1 + 40) = 9;
      __asm { vmovups ymmword ptr [r13+30h], ymm0 }
      *(_QWORD *)(a1 + 80) = 16;
      *(_BYTE *)(a1 + 88) = 1;
      *(_QWORD *)(a1 + 112) = 0x1FE00000000000LL;
      do
      {
        v10 = 1LL << v9;
        v11 = v9++ >> 6;
        *(_QWORD *)(a1 + 8 * v11 + 112) |= v10;
      }
      while ( v9 != 85 );
      *(_QWORD *)(a1 + 112) |= 0x1A0000007E57FC3uLL;
      break;
    case 9u:
    case 0xBu:
      __asm { vmovups xmm0, cs:xmmword_12B6570; jumptable 00000000006B99F8 cases 9,11 }
      v7 = 0x11FE00007F79CC3LL;
      *(_QWORD *)(a1 + 32) = 4;
      goto LABEL_10;
    case 0xAu:
      __asm { vmovups xmm0, cs:xmmword_12B6570; jumptable 00000000006B99F8 case 10 }
      v7 = 0x1201FFFFFF7FCC3LL;
      *(_QWORD *)(a1 + 32) = 8;
LABEL_10:
      *(_DWORD *)(a1 + 40) = 9;
      *(_QWORD *)(a1 + 48) = 15;
      __asm { vmovups xmmword ptr [r13+48h], xmm0 }
      goto LABEL_12;
    case 0xCu:
      __asm { vmovups ymm0, cs:ymmword_12B6580; jumptable 00000000006B99F8 case 12 }
      v7 = 0x1A01FFFFFF7FFC3LL;
      *(_QWORD *)(a1 + 32) = 8;
      *(_DWORD *)(a1 + 40) = 9;
      __asm { vmovups ymmword ptr [r13+30h], ymm0 }
      *(_QWORD *)(a1 + 80) = 16;
LABEL_12:
      *(_BYTE *)(a1 + 88) = 1;
      *(_QWORD *)(a1 + 112) = v7;
      break;
    default:
      break;
  }
  sub_256FD0(v60, byte_12B65C8);
  v13 = sub_363030(a2);
  v14 = sub_256FD0(&v59, "resolutions");
  v15 = sub_369A90(v60[0], v13, v59, v14);
  v16 = (_QWORD *)*_R13;
  v17 = *(__int16 *)(v15 + 20);
  v57 = v15;
  v18 = (unsigned int)(v17 - 1);
  if ( (_R13[2] - *_R13) >> 3 < v18 )
  {
    v16 = (_QWORD *)sub_252CF0(_R13 + 3, 8 * v18, 0);
    v19 = _R13[1] - *_R13;
    memmove(v16, *_R13, v19);
    v20 = (char *)v16 + v19;
    if ( *_R13 != 0 )
      sub_252D30(_R13 + 3, *_R13, _R13[2] - *_R13);
    *_R13 = (__int64)v16;
    _R13[1] = (__int64)v20;
    _R13[2] = (__int64)&v16[v18];
    LOWORD(v17) = *(_WORD *)(v15 + 20);
  }
  if ( ((__int16)v17 & 0xFFFFFFFE) != 0 )
  {
    v21 = "so.1";
    v22 = 1;
    v56 = _R13 + 3;
    do
    {
      v23 = sub_E850(&v21[*(_QWORD *)v15], v15);
      v58 = 0;
      if ( (unsigned __int8)render_parse_resolution(v23, &v58) != 0 )
      {
        v24 = (_QWORD *)_R13[1];
        if ( (unsigned __int64)v24 >= _R13[2] )
        {
          v25 = *_R13;
          v26 = ((__int64)v24 - *_R13) >> 2;
          if ( v24 == (_QWORD *)*_R13 )
            v26 = 1;
          if ( v26 != 0 )
          {
            v27 = sub_252CF0(v56, 8 * v26, 0);
            v25 = *_R13;
            v24 = (_QWORD *)_R13[1];
            v55 = v27;
          }
          else
          {
            v27 = 0;
            v55 = 0;
          }
          v28 = (char *)v24 - v25;
          v54 = v27;
          memmove(v27, v25, v28);
          *(_QWORD *)&v28[v55] = v58;
          v29 = (__int64)&v28[v55 + 8];
          if ( *_R13 != 0 )
          {
            sub_252D30(v56, *_R13, _R13[2] - *_R13);
            v29 = (__int64)&v28[v55 + 8];
          }
          *_R13 = v54;
          _R13[1] = v29;
          _R13[2] = v55 + 8 * v26;
          v15 = v57;
        }
        else
        {
          _R13[1] = (__int64)(v24 + 1);
          *v24 = v58;
        }
      }
      ++v22;
      v21 += 16;
    }
    while ( v22 < (unsigned int)*(__int16 *)(v15 + 20) );
    v16 = (_QWORD *)*_R13;
    v30 = _R13 + 1;
  }
  else
  {
    v30 = _R13 + 1;
  }
  v31 = *v30;
  if ( v16 == (_QWORD *)*v30 )
  {
    if ( (unsigned __int64)v16 >= _R13[2] )
    {
      v32 = v30;
      v33 = sub_252CF0(_R13 + 3, 8, 0);
      v34 = *v32 - *_R13;
      memmove(v33, *_R13, v34);
      v31 = v33 + v34 + 8;
      *(_QWORD *)(v33 + v34) = 0x43800000780LL;
      if ( *_R13 != 0 )
        sub_252D30(_R13 + 3, *_R13, _R13[2] - *_R13);
      *_R13 = v33;
      _R13[1] = v31;
      _R13[2] = v33 + 8;
    }
    else
    {
      *v30 = (__int64)(v16 + 1);
      *v16 = 0x43800000780LL;
      v31 = *v30;
    }
  }
  v35 = (_QWORD *)*_R13;
  if ( *_R13 != v31 )
  {
    v36 = -1;
    for ( i = (v31 - (__int64)v35) >> 3; i != 0; ++v36 )
      i >>= 1;
    sub_6BA000(*_R13, v31, 2LL * v36, i);
    if ( v31 - (__int64)v35 < 225 )
    {
      v48 = v35 + 1;
      if ( v35 + 1 != (_QWORD *)v31 )
      {
        v49 = 8;
        do
        {
          v50 = *v48;
          v51 = v35;
          if ( v48 != v35 )
          {
            v52 = v49;
            v51 = v48;
            do
            {
              if ( (_DWORD)v50 == *(_DWORD *)((char *)v35 + v52 - 8) )
              {
                if ( SHIDWORD(v50) >= *(_DWORD *)((char *)v35 + v52 - 4) )
                {
                  v51 = (_QWORD *)((char *)v35 + v52);
                  goto LABEL_70;
                }
              }
              else if ( (int)v50 >= *(_DWORD *)((char *)v35 + v52 - 8) )
              {
                goto LABEL_70;
              }
              --v51;
              *(_QWORD *)((char *)v35 + v52) = *(_QWORD *)((char *)v35 + v52 - 8);
              v52 -= 8;
            }
            while ( v52 != 0 );
            v51 = v35;
          }
LABEL_70:
          ++v48;
          v49 += 8;
          *v51 = v50;
        }
        while ( v48 != (_QWORD *)v31 );
      }
    }
    else
    {
      v38 = 1;
      v39 = 8;
      do
      {
        v40 = v35[v38];
        v41 = &v35[v38];
        v42 = v39;
        while ( 1 )
        {
          if ( (_DWORD)v40 == *(_DWORD *)((char *)v35 + v42 - 8) )
          {
            if ( SHIDWORD(v40) >= *(_DWORD *)((char *)v35 + v42 - 4) )
              goto LABEL_52;
            goto LABEL_49;
          }
          if ( (int)v40 >= *(_DWORD *)((char *)v35 + v42 - 8) )
            break;
LABEL_49:
          --v41;
          *(_QWORD *)((char *)v35 + v42) = *(_QWORD *)((char *)v35 + v42 - 8);
          v42 -= 8;
          if ( v42 == 0 )
          {
            v41 = v35;
            goto LABEL_52;
          }
        }
        v41 = (_QWORD *)((char *)v35 + v42);
LABEL_52:
        ++v38;
        v39 += 8;
        *v41 = v40;
      }
      while ( v38 != 28 );
      v43 = v35 + 28;
      if ( v43 != (_QWORD *)v31 )
      {
        while ( 1 )
        {
          v44 = *v43;
          v45 = v43;
          v46 = HIDWORD(*v43);
          v47 = (int)*v43 < *((_DWORD *)v43 - 2);
          if ( (unsigned int)*v43 != *((_DWORD *)v43 - 2) )
            break;
          while ( (int)v46 < *((_DWORD *)v45 - 1) )
          {
LABEL_56:
            *v45 = *(v45 - 1);
            v47 = (int)v44 < *((_DWORD *)--v45 - 2);
            if ( (_DWORD)v44 != *((_DWORD *)v45 - 2) )
              goto LABEL_57;
          }
LABEL_58:
          *v45 = v44;
          if ( ++v43 == (_QWORD *)v31 )
            return 0x6365786562696C2FLL;
        }
LABEL_57:
        if ( !v47 )
          goto LABEL_58;
        goto LABEL_56;
      }
    }
  }
  return 0x6365786562696C2FLL;
}
