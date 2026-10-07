__int64 __fastcall fmod_streaming_clip_initialize_buffers(__int64 a1, _DWORD *a2)
{
  int v4; // eax
  int v5; // edx
  double v8; // xmm0_8
  __int64 v9; // rax
  __int64 *v10; // r15
  _QWORD *v11; // r14
  unsigned __int64 v12; // rcx
  unsigned __int64 v13; // rdi
  __int64 v14; // rdx
  __int64 v15; // r12
  _QWORD *v16; // rbx
  __int64 v17; // rax
  _QWORD *v18; // rax
  _QWORD **v19; // rcx
  __int64 v20; // rdx
  _QWORD *v21; // rdx
  _QWORD *v22; // rdi
  int v23; // r13d
  __int64 v24; // r12
  __int64 v25; // r15
  int v26; // eax
  __int64 v27; // rdi
  int v28; // r14d
  __int64 v29; // rsi
  int v30; // r8d
  __int64 v32; // [rsp+0h] [rbp-90h]
  _DWORD *v33; // [rsp+8h] [rbp-88h]
  __int64 v34; // [rsp+10h] [rbp-80h]
  int v35; // [rsp+1Ch] [rbp-74h]
  _QWORD v36[2]; // [rsp+20h] [rbp-70h] BYREF
  void *v37; // [rsp+30h] [rbp-60h] BYREF
  int v38; // [rsp+38h] [rbp-58h]
  __int64 v39; // [rsp+40h] [rbp-50h]
  void **v40; // [rsp+50h] [rbp-40h]
  __int64 v41; // [rsp+60h] [rbp-30h]

  _RBX = a1;
  v41 = 0x6365786562696C2FLL;
  *(_BYTE *)(a1 + 492) = 1;
  *(_DWORD *)(a1 + 484) = 0;
  *(_DWORD *)(a1 + 488) = 0;
  *(_DWORD *)(a1 + 500) = 0;
  *(_DWORD *)(a1 + 476) = 1065353216;
  v4 = a2[28];
  *(_DWORD *)(a1 + 452) = v4;
  v5 = a2[26];
  *(_DWORD *)(a1 + 448) = v5;
  *(_DWORD *)(a1 + 504) = 0;
  *(_DWORD *)(a1 + 496) = v4;
  __asm { vmovsd  xmm0, qword ptr [rbx+0E8h] }
  __asm { vcvtsd2ss xmm0, xmm0, xmm0 }
  v8 = sub_D3D20(v36, 2, (unsigned int)(a2[27] * v5), 1, *(double *)&_XMM0);
  sub_135B0(_RBX + 320, v36[0], v36[1], 0, v8);
  v9 = *(_QWORD *)(_RBX + 280);
  v34 = _RBX;
  v10 = (__int64 *)(_RBX + 280);
  v11 = *(_QWORD **)(_RBX + 288);
  v33 = a2;
  v12 = (int)a2[27];
  v13 = 0xAAAAAAAAAAAAAAABLL * (((__int64)v11 - v9) >> 6);
  if ( v12 <= v13 )
  {
    v15 = 192 * v12;
    v16 = (_QWORD *)(v9 + 192 * v12);
    if ( v16 != v11 )
    {
      do
      {
        v17 = v16[23];
        if ( v17 != 0 )
        {
          v16[23] = 0;
          --*(_QWORD *)(v17 + 16);
          v18 = v16 + 21;
          v19 = (_QWORD **)(v16 + 22);
          v20 = v16[21];
          *(_QWORD *)(v20 + 8) = v16[22];
          *(_QWORD *)v16[22] = v20;
          v21 = v16 + 21;
          v16[21] = v16 + 21;
          v16[22] = v16 + 21;
        }
        else
        {
          v18 = (_QWORD *)v16[21];
          v21 = (_QWORD *)v16[22];
          v19 = (_QWORD **)(v16 + 22);
        }
        v18[1] = v21;
        **v19 = v18;
        v22 = (_QWORD *)v16[14];
        if ( v22 != nullptr )
        {
          (*(void (__fastcall **)(_QWORD *, bool))(*v22 + 32LL))(v22, v16 + 10 != v22);
          v16[14] = 0;
        }
        v16 += 24;
      }
      while ( v11 != v16 );
      v9 = *v10;
    }
    v14 = v34;
    *(_QWORD *)(v34 + 288) = v15 + v9;
    if ( *(_BYTE *)(v34 + 420) == 0 )
      goto LABEL_14;
LABEL_13:
    v35 = *(_DWORD *)(v14 + 412);
    goto LABEL_15;
  }
  sub_26E420(_RBX + 280, v12 - v13);
  v14 = _RBX;
  if ( *(_BYTE *)(_RBX + 420) != 0 )
    goto LABEL_13;
LABEL_14:
  v35 = 1;
LABEL_15:
  if ( (int)v33[27] > 0 )
  {
    v23 = 0;
    v24 = 0;
    v25 = 0;
    v32 = *(_QWORD *)(v14 + 344);
    v26 = *(_DWORD *)(v14 + 448);
    v27 = *(_QWORD *)(v14 + 280);
    v28 = -v26;
    do
    {
      v29 = *(_QWORD *)(v14 + 456);
      v30 = *(_DWORD *)(v14 + 428) * *(_DWORD *)(v14 + 424);
      v37 = &unk_18F07E8;
      v38 = v25;
      v39 = v14;
      v40 = &v37;
      sub_263EC0(v24 + v27, v29, 2, v32 + 2LL * v23 * v26, (unsigned int)(2 * v30), &v37);
      if ( v40 != nullptr )
      {
        (*((void (__fastcall **)(void **, bool))*v40 + 4))(v40, v40 != &v37);
        v40 = nullptr;
      }
      v14 = v34;
      ++v25;
      v27 = *(_QWORD *)(v34 + 280);
      v28 += *(_DWORD *)(v34 + 448);
      *(_DWORD *)(v27 + v24 + 136) = v28;
      *(_DWORD *)(v27 + v24) = v28;
      v26 = *(_DWORD *)(v34 + 448);
      *(_DWORD *)(v27 + v24 + 4) = v26;
      *(_DWORD *)(v27 + v24 + 12) = 0;
      v24 += 192;
      v23 += v35;
    }
    while ( v25 < (int)v33[27] );
  }
  return 0x6365786562696C2FLL;
}
