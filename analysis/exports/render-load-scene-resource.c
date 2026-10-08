_QWORD *__fastcall render_load_scene_resource(_QWORD *a1, __int64 a2, unsigned __int8 a3)
{
  __int64 v5; // rdx
  __int64 v6; // rbx
  __int64 v7; // rdi
  __int64 v10; // [rsp+10h] [rbp-40h] BYREF
  _QWORD v11[7]; // [rsp+18h] [rbp-38h] BYREF

  v11[1] = 0x6365786562696C2FLL;
  if ( byte_19C7F28[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19C7F28) != 0 )
  {
    unk_19C7F20 = 19246190;
    _cxa_guard_release(byte_19C7F28);
  }
  if ( unk_19C7F20 == 19246190 )
  {
    sub_256FD0(v11, "RndSceneResource");
    unk_19C7F20 = v11[0];
  }
  *a1 = 0;
  if ( byte_19C7F28[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19C7F28) != 0 )
  {
    unk_19C7F20 = 19246190;
    _cxa_guard_release(byte_19C7F28);
  }
  v5 = unk_19C7F20;
  if ( unk_19C7F20 == 19246190 )
  {
    sub_256FD0(v11, "RndSceneResource");
    v5 = v11[0];
    unk_19C7F20 = v11[0];
  }
  sub_1ABA50(&v10, a2, v5, a3);
  v6 = v10;
  if ( v10 != 0 )
    sub_1ADEB0(v10);
  if ( *a1 != 0 )
    sub_1ADEF0(*a1);
  v7 = v10;
  *a1 = v6;
  if ( v7 != 0 )
    sub_1ADEF0(v7);
  return a1;
}
