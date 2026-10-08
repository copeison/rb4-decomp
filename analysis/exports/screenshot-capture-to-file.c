// Renders the screenshot target and writes ../screenshots/screenshot.png.
__int64 __fastcall screenshot_capture_to_file(unsigned int *a1, int a2, int a3, __int64 a4)
{
  int v9; // eax
  unsigned int v10; // ecx
  unsigned int v11; // eax
  __int64 v12; // rax
  __int64 v13; // rdi
  __int64 v15; // rbx
  _QWORD *v16; // rdx
  __int64 v17; // rsi
  __int64 v18; // rbx
  __int64 v19; // r15
  double v20; // xmm0_8
  _BYTE *v22; // r15
  _BYTE *v23; // rbx
  unsigned __int64 v24; // rax
  __int64 v25; // rbx
  unsigned int v27[128]; // [rsp+0h] [rbp-6A8h] BYREF
  __m256 v28; // [rsp+200h] [rbp-4A8h] BYREF
  void *v44; // [rsp+410h] [rbp-298h] BYREF
  _BYTE *v45; // [rsp+418h] [rbp-290h]
  int v46; // [rsp+420h] [rbp-288h]
  _BYTE v47[124]; // [rsp+424h] [rbp-284h] BYREF
  _BYTE v48[392]; // [rsp+4A0h] [rbp-208h] BYREF
  _QWORD *v49; // [rsp+628h] [rbp-80h]
  __int128 v50; // [rsp+630h] [rbp-78h]
  _QWORD v51[13]; // [rsp+640h] [rbp-68h] BYREF

  v51[4] = 0x6365786562696C2FLL;
  v9 = *a1;
  if ( *a1 != 0 )
  {
    v10 = a1[1];
    if ( v10 != 0 )
    {
      byte_1A7306C[0] = 0;
      if ( v9 != unk_1A73088 || v10 != unk_1A7308C )
      {
        sub_690330(&v44);
        *(_QWORD *)&v47[116] = "Screenshot";
        *(_DWORD *)&v47[24] = 10;
        *(_QWORD *)&v47[16] = 0x100000001LL;
        sub_68DB80(v27, 9);
        v11 = sub_68E4D0(v27, 7u);
        sub_6830D0(v48, *a1, a1[1], 1, v11);
        v12 = sub_68FE80((__int64)&v44, 0);
        sub_6B0760((__int64)&unk_1A73070, v12, 0);
        sub_682BC0(v48);
      }
      unk_1A73080 = a2;
      unk_1A73084 = a3;
      sub_3DE8F0(g_render_system, &unk_1A73070);
      v13 = *(_QWORD *)(a4 + 32);
      if ( v13 == 0 )
      {
        std::_Xbad_function_call();
        BUG();
      }
      (*(void (__fastcall **)(__int64))(*(_QWORD *)v13 + 16LL))(v13);
      __asm { vmovups xmm0, cs:xmmword_1281690 }
      v15 = g_render_system;
      v16 = v51;
      v49 = v51;
      __asm { vmovups [rsp+6A8h+var_78], xmm0 }
      v51[0] = 0;
      v51[1] = unk_1A731E0;
      v51[2] = -1;
      v51[3] = 4;
      if ( *(_BYTE *)(g_render_system + 64) != 0 )
      {
        sub_3DEF20(g_render_system);
        v16 = v49;
        v17 = v50;
      }
      else
      {
        v17 = 1;
      }
      (*(void (__fastcall **)(_QWORD, __int64, _QWORD *))(**(_QWORD **)(v15 + 56) + 168LL))(
        *(_QWORD *)(v15 + 56),
        v17,
        v16);
      v18 = g_render_system;
      v19 = unk_1A731E0;
      if ( *(_BYTE *)(g_render_system + 64) != 0 )
        sub_3DEF20(g_render_system);
      sub_69B8D0(v19, *(_QWORD *)(v18 + 56));
      v20 = sub_3DE9E0(g_render_system);
      sub_6837C0(unk_1A731E0 + 312LL, &unk_1A73680, v20);
      __asm { vxorps  ymm0, ymm0, ymm0 }
      __asm
      {
        vmovups [rsp+6A8h+var_2C8], ymm0
        vmovups [rsp+6A8h+var_2E8], ymm0
        vmovups [rsp+6A8h+var_308], ymm0
        vmovups [rsp+6A8h+var_328], ymm0
        vmovups [rsp+6A8h+var_348], ymm0
        vmovups [rsp+6A8h+var_368], ymm0
        vmovups [rsp+6A8h+var_388], ymm0
        vmovups [rsp+6A8h+var_3A8], ymm0
        vmovups [rsp+6A8h+var_3C8], ymm0
        vmovups [rsp+6A8h+var_3E8], ymm0
        vmovups [rsp+6A8h+var_408], ymm0
        vmovups [rsp+6A8h+var_428], ymm0
        vmovups [rsp+6A8h+var_448], ymm0
        vmovups [rsp+6A8h+var_468], ymm0
        vmovups [rsp+6A8h+var_488], ymm0
        vmovups [rsp+6A8h+var_4A8], ymm0
      }
      v22 = (_BYTE *)sub_2456A0("../screenshots/screenshot.png", &v28);
      v44 = &unk_18E28E8;
      sub_258550(&v44);
      v45 = v47;
      v46 = 512;
      v47[0] = 0;
      v44 = &unk_18E28E8;
      v23 = v47;
      if ( v22 != nullptr )
      {
        v23 = v47;
        if ( *v22 != 0 )
        {
          v24 = strlen(v22);
          v25 = 512;
          if ( v24 < 0x200 )
            v25 = v24;
          memmove(v47, v22, v25);
          v23 = &v47[v25];
        }
      }
      *v23 = 0;
      sub_245300(v47, v27);
      sub_245530(v27, &byte_12816D5);
      sub_6822F0(&unk_1A73680, v45, 0, 2, 0);
    }
  }
  return 0x6365786562696C2FLL;
}
