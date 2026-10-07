__int64 *__fastcall fmod_bank_resource_bus_paths(__int64 *_RDI, __int64 a2, __m128 _XMM0)
{
  __int64 *v3; // r13
  FMOD::Studio::Bank *v6; // r15
  __int64 *v8; // r15
  int v9; // eax
  __int64 v10; // r14
  __int64 v11; // r15
  __int64 v12; // r12
  __int64 v13; // r12
  __int64 v14; // rax
  char v15; // cl
  __int64 v16; // r14
  __int64 *v17; // r13
  __int64 v18; // rcx
  __int64 *v19; // rsi
  __int64 *v20; // r12
  __int64 v21; // r15
  __int64 v22; // rax
  __int64 v23; // rbx
  char *v24; // r13
  __int64 v25; // r13
  __int64 v26; // rcx
  char v27; // al
  __int64 v28; // [rsp+0h] [rbp-4260h]
  __int64 *v29; // [rsp+8h] [rbp-4258h]
  __int64 *v30; // [rsp+10h] [rbp-4250h]
  __int64 v31; // [rsp+18h] [rbp-4248h] BYREF
  int v32; // [rsp+24h] [rbp-423Ch] BYREF
  int v33; // [rsp+28h] [rbp-4238h] BYREF
  int v34; // [rsp+2Ch] [rbp-4234h] BYREF
  char v35[512]; // [rsp+30h] [rbp-4230h] BYREF
  FMOD::Studio::Bus *v36[2054]; // [rsp+230h] [rbp-4030h] BYREF

  v3 = _RDI + 3;
  __asm { vxorps  xmm0, xmm0, xmm0 }
  v36[2048] = (FMOD::Studio::Bus *)0x6365786562696C2FLL;
  __asm { vmovups xmmword ptr [rdi], xmm0 }
  v29 = _RDI;
  _RDI[2] = 0;
  nullsub_18(_RDI + 3, "EASTL vector");
  if ( *(_QWORD *)(a2 + 88) != 0 )
  {
    v6 = *(FMOD::Studio::Bank **)(*(_QWORD *)(a2 + 64) + 40LL);
    if ( (unsigned int)FMOD::Studio::Bank::getBusCount(v6, &v34) == 0 )
    {
      FMOD::Studio::Bank::getBusList(v6, v36, 2048, &v33);
      v8 = v29;
      v9 = v33;
      v10 = v33;
      if ( (v29[2] - *v29) >> 3 < (unsigned __int64)v33 )
      {
        v11 = sub_252CF0(v3, 8LL * v33, 0);
        v12 = v29[1] - *v29;
        memmove(v11, *v29, v12);
        v13 = v11 + v12;
        if ( *v29 != 0 )
          sub_252D30(v3, *v29, v29[2] - *v29);
        v14 = v11 + 8 * v10;
        *v29 = v11;
        v29[1] = v13;
        v8 = v29;
        v29[2] = v14;
        v9 = v33;
      }
      v15 = 1;
      if ( v9 <= 0 )
      {
        v27 = 0;
      }
      else
      {
        v16 = 0;
        v30 = v3;
        while ( (unsigned int)FMOD::Studio::Bus::getPath(v36[v16], v35, 512, &v32) == 0 )
        {
          sub_256FD0(&v31, v35);
          v17 = (__int64 *)v8[1];
          if ( (unsigned __int64)v17 >= v8[2] )
          {
            v19 = (__int64 *)*v8;
            v20 = v8;
            v21 = ((__int64)v17 - *v8) >> 2;
            if ( v17 == v19 )
              v21 = 1;
            if ( v21 != 0 )
            {
              v22 = sub_252CF0(v30, 8 * v21, 0);
              v19 = (__int64 *)*v20;
              v17 = (__int64 *)v20[1];
              v23 = v22;
            }
            else
            {
              v22 = 0;
              v23 = 0;
            }
            v24 = (char *)((char *)v17 - (char *)v19);
            v28 = v22;
            memmove(v22, v19, v24);
            *(_QWORD *)&v24[v23] = v31;
            v25 = (__int64)&v24[v23 + 8];
            if ( *v20 != 0 )
              sub_252D30(v30, *v20, v20[2] - *v20);
            v26 = v23 + 8 * v21;
            v8 = v20;
            *v20 = v28;
            v20[1] = v25;
            v20[2] = v26;
          }
          else
          {
            v18 = v31;
            v8[1] = (__int64)(v17 + 1);
            *v17 = v18;
          }
          if ( ++v16 >= v33 )
          {
            v27 = 0;
            v15 = 1;
            goto LABEL_25;
          }
        }
        v27 = 1;
        v15 = 0;
LABEL_25:
        v3 = v30;
      }
      if ( v15 == 0 && v27 == 0 && *v29 != 0 )
        sub_252D30(v3, *v29, v29[2] - *v29);
    }
  }
  return v29;
}
