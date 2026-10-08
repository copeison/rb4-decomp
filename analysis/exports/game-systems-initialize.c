// Initializes the primary game systems from a startup option block and registers their cleanup callback.
void __fastcall game_systems_initialize(const void *options)
{
  __int64 v2; // rdx
  __int64 v3; // rdi
  __int64 v4; // rsi
  __int64 v5; // rdx
  __int64 v6; // rdi
  __int64 v7; // rsi
  __int64 v8; // rdx
  __int64 v9; // rdi
  __int64 v10; // rsi
  __int64 v11; // rdx
  __int64 v12; // rdi
  __int64 v13; // rsi
  __int64 v14; // rdx
  __int64 v15; // rdi
  __int64 v16; // rsi
  __int64 v17; // rdx
  __int64 v18; // rdi
  __int64 v19; // rsi
  __int64 v20; // rdx
  __int64 v21; // rdi
  __int64 v22; // rsi
  __int64 v23; // rdx
  __int64 v24; // rdi
  __int64 v25; // rsi
  __int64 v26; // rdx
  __int64 v27; // rdi
  __int64 v28; // rsi
  __int64 v29; // rdx
  __int64 v30; // rdi
  __int64 v31; // rsi
  __int64 v32; // rbx
  _QWORD *v33; // rax
  __int128 v34; // [rsp+0h] [rbp-30h] BYREF
  __int64 v35; // [rsp+10h] [rbp-20h]

  v35 = 0x6365786562696C2FLL;
  __asm
  {
    vmovups xmm0, xmmword ptr [rdi]
    vmovups [rbp+var_30], xmm0
  }
  orbis_render_system_create(options);
  sub_8D5DE0(v3, v4, v2);
  sub_4414A0(7);
  render_system_initialize(g_render_system, (__int64)&v34);
  render_initialize_default_resources((__int64 *)(g_render_system + 1976), (__int64)&v34);
  sub_47F8D0((_QWORD *)(g_render_system + 3256), (__int64)&v34);
  sub_6B54A0(v6, v7, v5);
  sub_65C1A0(v9, v10, v8);
  sub_3DF170(v12, v13, v11);
  sub_461D10(v15, v16, v14);
  sub_5F7ED0(v18, v19, v17);
  sub_5F9150(v21, v22, v20);
  sub_601880(v24, v25, v23);
  sub_44C8B0(v27, v28, v26);
  sub_65CAF0(v30, v31, v29);
  v32 = qword_19FDB88[0];
  v33 = (_QWORD *)sub_252CF0(&unk_19FDBA0, 24, 0);
  v33[2] = game_systems_shutdown;
  *v33 = v32;
  v33[1] = *(_QWORD *)(v32 + 8);
  **(_QWORD **)(v32 + 8) = v33;
  *(_QWORD *)(v32 + 8) = v33;
  ++unk_19FDB98;
}
