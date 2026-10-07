__int64 __fastcall fmod_audio_stream_resource_load(__int64 a1)
{
  _BYTE *v3; // r12
  __int64 v4; // rax
  _BYTE *v6; // r12
  _BYTE *v7; // r15
  _BYTE *v8; // rbx
  unsigned __int64 v9; // rax
  __int64 v10; // rbx
  char *v11; // rbx
  _BYTE *v12; // rbx
  unsigned __int64 v13; // rax
  __int64 v14; // rbx
  unsigned __int64 v15; // rbx
  __int64 v16; // rax
  double v17; // xmm0_8
  __int64 v18; // rdi
  __int64 v19; // rdi
  int v21; // [rsp+18h] [rbp-8D0h] BYREF
  int v22; // [rsp+1Ch] [rbp-8CCh] BYREF
  int v23; // [rsp+20h] [rbp-8C8h] BYREF
  _BYTE v24[4]; // [rsp+24h] [rbp-8C4h] BYREF
  __int64 v25; // [rsp+28h] [rbp-8C0h] BYREF
  FMOD::Sound *v26; // [rsp+30h] [rbp-8B8h] BYREF
  __int64 v27; // [rsp+38h] [rbp-8B0h] BYREF
  __int64 v28; // [rsp+40h] [rbp-8A8h] BYREF
  _QWORD v29[2]; // [rsp+48h] [rbp-8A0h] BYREF
  int v30; // [rsp+58h] [rbp-890h]
  _BYTE v31[516]; // [rsp+5Ch] [rbp-88Ch] BYREF
  __m256 v32; // [rsp+260h] [rbp-688h] BYREF
  _QWORD v48[2]; // [rsp+470h] [rbp-478h] BYREF
  int v49; // [rsp+480h] [rbp-468h]
  _BYTE v50[516]; // [rsp+484h] [rbp-464h] BYREF
  void *v51; // [rsp+688h] [rbp-260h] BYREF
  char *v52; // [rsp+690h] [rbp-258h]
  int v53; // [rsp+698h] [rbp-250h]
  char v54; // [rsp+69Ch] [rbp-24Ch] BYREF
  __int64 v55; // [rsp+8A0h] [rbp-48h]

  v55 = 0x6365786562696C2FLL;
  *(_QWORD *)(a1 + 56) = 3212836864LL;
  v3 = *(_BYTE **)(a1 + 8);
  v51 = &unk_18E28E8;
  sub_258550(&v51);
  v52 = &v54;
  v53 = 512;
  v54 = 0;
  v51 = &unk_18E28E8;
  if ( *(_BYTE *)sub_377250() != 0 )
  {
    v4 = sub_377250();
    __asm { vxorps  ymm0, ymm0, ymm0 }
    __asm
    {
      vmovups [rsp+8E8h+var_4A8], ymm0
      vmovups [rsp+8E8h+var_4C8], ymm0
      vmovups [rsp+8E8h+var_4E8], ymm0
      vmovups [rsp+8E8h+var_508], ymm0
      vmovups [rsp+8E8h+var_528], ymm0
      vmovups [rsp+8E8h+var_548], ymm0
      vmovups [rsp+8E8h+var_568], ymm0
      vmovups [rsp+8E8h+var_588], ymm0
      vmovups [rsp+8E8h+var_5A8], ymm0
      vmovups [rsp+8E8h+var_5C8], ymm0
      vmovups [rsp+8E8h+var_5E8], ymm0
      vmovups [rsp+8E8h+var_608], ymm0
      vmovups [rsp+8E8h+var_628], ymm0
      vmovups [rsp+8E8h+var_648], ymm0
      vmovups [rsp+8E8h+var_668], ymm0
      vmovups [rsp+8E8h+var_688], ymm0
    }
    v6 = (_BYTE *)sub_244960(v4, v3, &v32);
    v48[0] = &unk_18E28E8;
    sub_258550(v48);
    v7 = v50;
    v48[1] = v50;
    v49 = 512;
    v50[0] = 0;
    v48[0] = &unk_18E28E8;
    v8 = v50;
    if ( v6 != nullptr )
    {
      v8 = v50;
      if ( *v6 != 0 )
      {
        v9 = strlen(v6);
        v10 = 512;
        if ( v9 < 0x200 )
          v10 = v9;
        memmove(v50, v6, v10);
        v8 = &v50[v10];
      }
    }
    *v8 = 0;
    if ( v50[0] == 0 )
    {
      v11 = v52;
      goto LABEL_18;
    }
  }
  else
  {
    v29[0] = &unk_18E28E8;
    sub_258550(v29);
    v7 = v31;
    v29[1] = v31;
    v30 = 512;
    v31[0] = 0;
    v29[0] = &unk_18E28E8;
    v12 = v31;
    if ( v3 != nullptr )
    {
      v12 = v31;
      if ( *v3 != 0 )
      {
        v13 = strlen(v3);
        v14 = 512;
        if ( v13 < 0x200 )
          v14 = v13;
        memmove(v31, v3, v14);
        v12 = &v31[v14];
      }
    }
    *v12 = 0;
    if ( v31[0] == 0 )
    {
      v11 = v52;
      goto LABEL_18;
    }
  }
  v15 = strlen(v7);
  if ( *((unsigned int *)v52 - 1) < v15 )
    v15 = *((unsigned int *)v52 - 1);
  memmove(v52, v7, v15);
  v11 = &v52[v15];
LABEL_18:
  *v11 = 0;
  v16 = sub_2448D0(v52);
  v17 = sub_256FD0(&v28, v16);
  v18 = v28;
  *(_QWORD *)(a1 + 48) = v28;
  if ( (unsigned __int8)sub_3788B0(v18, 2, v17) != 0 )
  {
    v26 = nullptr;
    v19 = *(_QWORD *)(unk_19F29D8 + 288LL);
    if ( v19 != 0 && (FMOD::System::createSound(v19, *(_QWORD *)(a1 + 48), 128, 0, &v26), v26 != nullptr) )
    {
      FMOD::Sound::getFormat(v26, v24, &v23, &v22, &v21);
      if ( v23 != 2 || (unsigned int)(v22 - 1) > 1 || v21 != 16 )
        *(_DWORD *)(a1 + 60) = 3;
      FMOD::Sound::release(v26);
      fmod_audio_stream_resource_register(a1);
    }
    else
    {
      sub_256FD0(&v25, 19246190);
      *(_QWORD *)(a1 + 48) = v25;
      *(_DWORD *)(a1 + 60) = 2;
    }
  }
  else
  {
    sub_256FD0(&v27, 19246190);
    *(_QWORD *)(a1 + 48) = v27;
    *(_DWORD *)(a1 + 60) = 1;
  }
  return 0x6365786562696C2FLL;
}
