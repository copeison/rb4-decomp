// Chooses a capture extent and captures the supplied frame owner.
__int64 __fastcall screenshot_capture_frame(__int64 a1, signed int a2, __int64 a3)
{
  __int64 v5; // rax
  unsigned __int64 v6; // rcx
  unsigned __int64 v7; // rax
  int v8; // r15d
  int v9; // eax
  __int64 (__fastcall ***v10)(_QWORD, _BYTE *); // rdi
  int v11; // ebx
  __int64 v12; // rax
  _BYTE *v13; // rsi
  unsigned __int64 v15; // [rsp+8h] [rbp-68h] BYREF
  _BYTE v16[32]; // [rsp+10h] [rbp-60h] BYREF
  _BYTE *v17; // [rsp+30h] [rbp-40h]
  __int64 v18; // [rsp+48h] [rbp-28h]

  v18 = 0x6365786562696C2FLL;
  v15 = 0;
  if ( a2 != 0 )
  {
    if ( (unsigned int)a2 > 5 )
    {
      v6 = 0xFFFFFFFF00000000LL;
      v5 = 0xFFFFFFFFLL;
    }
    else
    {
      v5 = *((unsigned int *)qword_1281700 + a2);
      v6 = (unsigned __int64)*((unsigned int *)qword_1281720 + a2) << 32;
    }
    v7 = v6 | v5;
  }
  else
  {
    v7 = render_frame_owner_output_extent(a1);
  }
  v15 = v7;
  v8 = render_frame_owner_draw_mode(a1);
  v9 = render_frame_owner_debug_view(a1);
  v10 = *(__int64 (__fastcall ****)(_QWORD, _BYTE *))(a3 + 32);
  v11 = v9;
  v12 = 0;
  if ( v10 != nullptr )
  {
    v13 = v16;
    if ( v10 != (__int64 (__fastcall ***)(_QWORD, _BYTE *))a3 )
      v13 = nullptr;
    v12 = (**v10)(v10, v13);
  }
  v17 = (_BYTE *)v12;
  screenshot_capture_to_file((unsigned int *)&v15, v8, v11, (__int64)v16);
  if ( v17 != nullptr )
  {
    (*(void (__fastcall **)(_BYTE *, bool))(*(_QWORD *)v17 + 32LL))(v17, v17 != v16);
    v17 = nullptr;
  }
  return 0x6365786562696C2FLL;
}
