__int64 fmod_load_modules_and_set_thread_affinity()
{
  const char *v0; // rsi
  __int64 v1; // rbx
  __int64 v2; // r15
  __int64 v3; // r15
  int v5; // eax
  __int64 v9; // [rsp+0h] [rbp-80h] BYREF
  __int64 v10; // [rsp+8h] [rbp-78h] BYREF
  __int64 v11; // [rsp+10h] [rbp-70h] BYREF
  const char *v12; // [rsp+18h] [rbp-68h]
  __int128 v13; // [rsp+20h] [rbp-60h] BYREF
  int v14; // [rsp+30h] [rbp-50h]
  int v15; // [rsp+34h] [rbp-4Ch]
  int v16; // [rsp+38h] [rbp-48h]
  __int64 v18; // [rsp+58h] [rbp-28h]

  v18 = 0x6365786562696C2FLL;
  sub_255080(&v11);
  v0 = "/app0/%s.prx";
  if ( v12 != "/app0/%s.prx" )
  {
    v1 = 12;
    (*(void (__fastcall **)(__int64 *, __int64))(v11 + 24))(&v11, 12);
    if ( *((unsigned int *)v12 - 1) < 0xCuLL )
      v1 = *((unsigned int *)v12 - 1);
    memcpy(v12, "/app0/%s.prx", v1);
    v12[v1] = 0;
    v0 = v12;
  }
  sub_2472C0(&v13, v0);
  sub_247F90(&v13, "libfmod");
  v2 = sub_2484E0(&v13);
  sub_247510(&v13);
  sceKernelLoadStartModule(v2, 0, 0, 0, 0, &v10);
  sub_2472C0(&v13, v12);
  sub_247F90(&v13, "libfmodstudio");
  v3 = sub_2484E0(&v13);
  sub_247510(&v13);
  sceKernelLoadStartModule(v3, 0, 0, 0, 0, &v10);
  sub_258C60("audio_render", &v10);
  sub_258C60("mic_reader", &v9);
  _EBX = sub_2590B0(*(_QWORD *)(v10 + 24), *(_QWORD *)(v10 + 40));
  v5 = sub_2590B0(*(_QWORD *)(v9 + 24), *(_QWORD *)(v9 + 40));
  __asm { vmovd   xmm0, ebx }
  __asm
  {
    vpshufd xmm0, xmm0, 0
    vmovdqu [rbp+var_60], xmm0
  }
  v14 = _EBX;
  v15 = _EBX;
  v16 = v5;
  __asm { vmovdqu [rbp+var_44], xmm0 }
  FMOD_Orbis_SetThreadAffinity(&v13);
  sub_255550(&v11);
  return 0x6365786562696C2FLL;
}
