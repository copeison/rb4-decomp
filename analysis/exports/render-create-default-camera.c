__int64 __fastcall render_create_default_camera(__int64 a1, __int64 a2)
{
  _QWORD *v3; // rbx
  __int64 v5[5]; // [rsp+8h] [rbp-28h] BYREF

  v5[1] = 0x6365786562696C2FLL;
  v3 = (_QWORD *)sub_F0A10(a2, 0, 0);
  sub_256FD0(v5, "default_cam");
  sub_1160F0(v3, v5[0]);
  *(_QWORD *)(a1 + 416) = sub_117700(v3, unk_1AAF8F8, 0);
  return 0x6365786562696C2FLL;
}
