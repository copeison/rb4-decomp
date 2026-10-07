// Consumes CPU and engine timing statistics for every registered audio system; inferred telemetry maintenance path.
void __fastcall audio_consume_all_timing_reports()
{
  __m256 *v5; // rdx
  __m256 *v6; // rcx
  __m256 *v7; // r8
  FMOD::System ***v8; // r13
  FMOD::System ***v9; // rbx
  FMOD::System **v10; // r15
  __int64 v11; // r12
  __int128 *p_source_keys; // rax
  __int64 v13; // rax
  double *v14; // rbx
  double *v15; // r12
  __m256 *v16; // rsi
  double v17; // rax
  __m256 *v18; // rcx
  __m256 *v19; // rdx
  __m256 *v20; // rsi
  __int64 v21; // rdx
  __int64 v22; // rcx
  __int64 v23; // r8
  double buffer_set_maximum_percent; // [rsp+8h] [rbp-138h] BYREF
  double buffer_set_average_percent; // [rsp+10h] [rbp-130h] BYREF
  double reserved_maximum_2; // [rsp+18h] [rbp-128h] BYREF
  double reserved_average_2; // [rsp+20h] [rbp-120h] BYREF
  double reserved_maximum_1; // [rsp+28h] [rbp-118h] BYREF
  double reserved_average_1; // [rsp+30h] [rbp-110h] BYREF
  double fmod_maximum_percent; // [rsp+38h] [rbp-108h] BYREF
  double fmod_average_percent; // [rsp+40h] [rbp-100h] BYREF
  double engine_maximum_percent; // [rsp+48h] [rbp-F8h] BYREF
  int v33; // [rsp+54h] [rbp-ECh] BYREF
  int v34; // [rsp+58h] [rbp-E8h] BYREF
  int v35; // [rsp+5Ch] [rbp-E4h] BYREF
  int v36; // [rsp+60h] [rbp-E0h] BYREF
  int v37; // [rsp+64h] [rbp-DCh] BYREF
  char v38[8]; // [rsp+68h] [rbp-D8h] BYREF
  char v39[8]; // [rsp+70h] [rbp-D0h] BYREF
  double engine_average_percent; // [rsp+78h] [rbp-C8h] BYREF
  char source_maximum_percent[8]; // [rsp+80h] [rbp-C0h] BYREF
  __m256 v42; // [rsp+88h] [rbp-B8h] BYREF
  __int64 v43; // [rsp+A8h] [rbp-98h]
  char v44[8]; // [rsp+B0h] [rbp-90h] BYREF
  char source_average_percent[8]; // [rsp+B8h] [rbp-88h] BYREF
  __m256 v46; // [rsp+C0h] [rbp-80h] BYREF
  __int64 v47; // [rsp+E0h] [rbp-60h]
  char v48[8]; // [rsp+E8h] [rbp-58h] BYREF
  __int128 source_keys; // [rsp+F0h] [rbp-50h] BYREF
  __int64 v50; // [rsp+100h] [rbp-40h]
  char v51[8]; // [rsp+108h] [rbp-38h] BYREF
  __int64 v52; // [rsp+110h] [rbp-30h]

  __asm { vxorps  xmm0, xmm0, xmm0 }
  v52 = 0x6365786562696C2FLL;
  __asm { vmovups [rbp+source_keys], xmm0 }
  v50 = 0;
  nullsub_18(v51, "EASTL vector");
  nullsub_18(v39, "EASTL map");
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups [rbp+var_80], ymm0 }
  v47 = 0;
  nullsub_19(v48, v39);
  *(_QWORD *)v46.m256_f32 = &v46;
  *(_OWORD *)&v46.m256_f32[2] = (unsigned __int64)&v46;
  LOBYTE(v46.m256_f32[6]) = 0;
  v47 = 0;
  nullsub_18(v38, "EASTL map");
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups [rbp+var_B8], ymm0 }
  v43 = 0;
  nullsub_19(v44, v38);
  *(_QWORD *)v42.m256_f32 = &v42;
  *(_OWORD *)&v42.m256_f32[2] = (unsigned __int64)&v42;
  LOBYTE(v42.m256_f32[6]) = 0;
  v43 = 0;
  v8 = (FMOD::System ***)unk_19F2F20;
  v9 = (FMOD::System ***)unk_19F2F28;
  if ( unk_19F2F20 != unk_19F2F28 )
  {
    do
    {
      v10 = *v8;
      v11 = (*((__int64 (__fastcall **)(FMOD::System **))**v8 + 16))(*v8);
      scePthreadMutexLock(v11 + 40);
      ++*(_DWORD *)(v11 + 32);
      v37 = 0;
      v36 = 0;
      v35 = 0;
      v34 = 0;
      v33 = 0;
      FMOD::System::getCPUUsage(v10[36], (float *)&v37, (float *)&v36, (float *)&v35, (float *)&v34, (float *)&v33);
      engine_average_percent = 0.0;
      engine_maximum_percent = 0.0;
      fmod_average_percent = 0.0;
      fmod_maximum_percent = 0.0;
      reserved_average_1 = 0.0;
      reserved_maximum_1 = 0.0;
      reserved_average_2 = 0.0;
      reserved_maximum_2 = 0.0;
      buffer_set_average_percent = 0.0;
      buffer_set_maximum_percent = 0.0;
      p_source_keys = nullptr;
      if ( (_QWORD)source_keys == *((_QWORD *)&source_keys + 1) )
        p_source_keys = &source_keys;
      fmod_audio_consume_timing_report(
        v10,
        &engine_average_percent,
        &engine_maximum_percent,
        &fmod_average_percent,
        &fmod_maximum_percent,
        &reserved_average_1,
        &reserved_maximum_1,
        &reserved_average_2,
        &reserved_maximum_2,
        &buffer_set_average_percent,
        &buffer_set_maximum_percent,
        source_average_percent,
        source_maximum_percent,
        p_source_keys);
      v13 = (*((__int64 (__fastcall **)(FMOD::System **))*v10 + 16))(v10);
      --*(_DWORD *)(v13 + 32);
      scePthreadMutexUnlock(v13 + 40);
      ++v8;
    }
    while ( v9 != v8 );
  }
  if ( byte_19F29F8 == 0 )
    byte_19F29F8 = 1;
  v15 = *((double **)&source_keys + 1);
  v14 = (double *)source_keys;
  if ( (_QWORD)source_keys != *((_QWORD *)&source_keys + 1) )
  {
    v7 = &v42;
    do
    {
      v16 = *(__m256 **)&v42.m256_f32[4];
      v17 = *v14;
      engine_maximum_percent = *v14;
      if ( *(_QWORD *)&v42.m256_f32[4] != 0 )
      {
        v18 = &v42;
        while ( 2 )
        {
          v19 = v16;
          while ( *(_QWORD *)v19[1].m256_f32 < *(_QWORD *)&v17 )
          {
            v19 = *(__m256 **)v19->m256_f32;
            if ( v19 == nullptr )
            {
              v19 = v18;
              if ( v18 == &v42 )
                goto LABEL_20;
              goto LABEL_17;
            }
          }
          v16 = *(__m256 **)&v19->m256_f32[2];
          v18 = v19;
          if ( v16 != nullptr )
            continue;
          break;
        }
        if ( v19 == &v42 )
          goto LABEL_20;
LABEL_17:
        if ( *(_QWORD *)&v17 >= *(_QWORD *)v19[1].m256_f32 )
          goto LABEL_22;
      }
      else
      {
LABEL_20:
        v19 = &v42;
      }
      sub_263CC0(&engine_average_percent, source_maximum_percent, v19, &engine_maximum_percent);
      v7 = &v42;
LABEL_22:
      v20 = *(__m256 **)&v46.m256_f32[4];
      if ( *(_QWORD *)&v46.m256_f32[4] != 0 )
      {
        v6 = &v46;
        while ( 2 )
        {
          v5 = v20;
          while ( *(_QWORD *)v5[1].m256_f32 < *(_QWORD *)&engine_maximum_percent )
          {
            v5 = *(__m256 **)v5->m256_f32;
            if ( v5 == nullptr )
            {
              v5 = v6;
              if ( v6 == &v46 )
                goto LABEL_33;
              goto LABEL_30;
            }
          }
          v20 = *(__m256 **)&v5->m256_f32[2];
          v6 = v5;
          if ( v20 != nullptr )
            continue;
          break;
        }
        if ( v5 == &v46 )
          goto LABEL_33;
LABEL_30:
        if ( *(_QWORD *)&engine_maximum_percent >= *(_QWORD *)v5[1].m256_f32 )
          goto LABEL_35;
      }
      else
      {
LABEL_33:
        v5 = &v46;
      }
      sub_263CC0(&engine_average_percent, source_average_percent, v5, &engine_maximum_percent);
      v7 = &v42;
LABEL_35:
      ++v14;
    }
    while ( v14 != v15 );
  }
  sub_2634B0(source_maximum_percent, *(_QWORD *)&v42.m256_f32[4], v5, v6, v7);
  sub_2634B0(source_average_percent, *(_QWORD *)&v46.m256_f32[4], v21, v22, v23);
  if ( (_QWORD)source_keys != 0 )
    sub_252D30(v51, source_keys, v50 - source_keys);
}
