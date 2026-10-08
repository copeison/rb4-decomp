// Constructs the base render system, its 13 platform configurations, default resources, backend state, singleton, and settings.
__int64 __fastcall render_system_construct(__int64 a1)
{
  __m128 v11; // xmm0
  __m128 v12; // xmm0
  __m128 v13; // xmm0
  __m128 v14; // xmm0
  __m128 v15; // xmm0
  __m128 v16; // xmm0
  __m128 v17; // xmm0
  __m128 v18; // xmm0
  __m128 v19; // xmm0
  __m128 v20; // xmm0
  __m128 v21; // xmm0
  __m128 v22; // xmm0
  unsigned int *v28; // rbx
  unsigned int *v29; // r14
  __int64 v30; // rbx
  char v32[8]; // [rsp+8h] [rbp-58h] BYREF
  unsigned int *v33; // [rsp+10h] [rbp-50h] BYREF
  unsigned int *v34; // [rsp+18h] [rbp-48h]
  __int64 v35; // [rsp+20h] [rbp-40h]
  char v36[8]; // [rsp+28h] [rbp-38h] BYREF
  __int64 v37; // [rsp+30h] [rbp-30h]

  _R13 = a1;
  v37 = 0x6365786562696C2FLL;
  *(_QWORD *)a1 = &unk_18FF528;
  *(_DWORD *)(a1 + 8) = 0;
  scePthreadMutexattrInit(v32);
  scePthreadMutexattrSettype(v32, 2);
  scePthreadMutexInit(_R13 + 16, v32, "hx crit sec");
  *(double *)&_XMM0 = scePthreadMutexattrDestroy(v32);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *(_QWORD *)(_R13 + 24) = 0;
  *(_BYTE *)(_R13 + 32) = 0;
  *(_BYTE *)(_R13 + 40) = 1;
  *(_BYTE *)(_R13 + 41) = 1;
  *(_BYTE *)(_R13 + 42) = 1;
  __asm { vmovups xmmword ptr [r13+30h], xmm0 }
  *(_BYTE *)(_R13 + 64) = 0;
  __asm
  {
    vmovups xmmword ptr [r13+50h], xmm0
    vmovups xmmword ptr [r13+44h], xmm0
  }
  nullsub_18(_R13 + 96, "EASTL vector");
  __asm { vxorps  ymm0, ymm0, ymm0 }
  *(_DWORD *)(_R13 + 104) = 0;
  __asm { vmovups ymmword ptr [r13+70h], ymm0 }
  *(_QWORD *)(_R13 + 144) = 0;
  nullsub_18(_R13 + 152, "EASTL vector");
  __asm { vxorps  xmm1, xmm1, xmm1 }
  __asm { vmovups xmmword ptr [r13+0A0h], xmm1 }
  *(_WORD *)(_R13 + 176) = 0;
  *(_QWORD *)(_R13 + 184) = _R13 + 208;
  _RAX = 6;
  __asm
  {
    vmovq   xmm0, rax
    vpslldq xmm0, xmm0, 8
    vmovdqu xmmword ptr [r13+0C0h], xmm0
    vmovups xmmword ptr [r13+100h], xmm1
  }
  *(_DWORD *)(_R13 + 272) = 0;
  *(_QWORD *)(_R13 + 280) = -1;
  __asm { vmovups xmmword ptr [r13+120h], xmm1 }
  *(_QWORD *)(_R13 + 304) = 0;
  render_platform_config_construct(_R13 + 312, _XMM0);
  render_platform_config_construct(_R13 + 440, v11);
  render_platform_config_construct(_R13 + 568, v12);
  render_platform_config_construct(_R13 + 696, v13);
  render_platform_config_construct(_R13 + 824, v14);
  render_platform_config_construct(_R13 + 952, v15);
  render_platform_config_construct(_R13 + 1080, v16);
  render_platform_config_construct(_R13 + 1208, v17);
  render_platform_config_construct(_R13 + 1336, v18);
  render_platform_config_construct(_R13 + 1464, v19);
  render_platform_config_construct(_R13 + 1592, v20);
  render_platform_config_construct(_R13 + 1720, v21);
  render_platform_config_construct(_R13 + 1848, v22);
  render_construct_default_resources((_QWORD *)(_R13 + 1976));
  sub_63F180(_R13 + 2544);
  sub_47EEE0(_R13 + 3256);
  *(double *)&_XMM0 = sub_451C70(_R13 + 3560);
  __asm
  {
    vpxor   xmm0, xmm0, xmm0
    vmovdqu xmmword ptr [r13+0DF0h], xmm0
  }
  sub_62AAE0(_R13 + 3584);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm { vmovups ymmword ptr [r13+0E80h], ymm0 }
  *(_DWORD *)(_R13 + 3744) = 0;
  scePthreadMutexattrInit(v32);
  scePthreadMutexattrSettype(v32, 2);
  scePthreadMutexInit(_R13 + 3752, v32, "hx crit sec");
  *(double *)&_XMM0 = scePthreadMutexattrDestroy(v32);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [r13+0EB0h], xmm0 }
  *(_QWORD *)(_R13 + 3776) = 0;
  nullsub_18(_R13 + 3784, "EASTL vector");
  *(_QWORD *)(_R13 + 3792) = 0;
  *(_DWORD *)(_R13 + 3800) = 0;
  g_render_system = _R13;
  render_supported_platform_ids((__int64 *)&v33);
  v28 = v33;
  v29 = v34;
  if ( v33 != v34 )
  {
    do
    {
      render_platform_config_initialize(_R13 + ((unsigned __int64)*v28 << 7) + 312, *v28);
      ++v28;
    }
    while ( v29 != v28 );
    v28 = v33;
  }
  if ( v28 != nullptr )
    sub_252D30(v36, v28, v35 - (_QWORD)v28);
  sub_6B9FE0(_R13 + 1208);
  v30 = sub_37BF40(232);
  render_settings_initialize(v30);
  *(_QWORD *)(_R13 + 296) = v30;
  return 0x6365786562696C2FLL;
}
