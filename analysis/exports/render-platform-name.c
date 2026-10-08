// Returns the interned name for one of the thirteen platform IDs.
__int64 __fastcall render_platform_name(unsigned int a1)
{
  int v2; // eax
  char *v3; // rax

  if ( byte_19FE188 == 0 )
  {
    _cxa_guard_acquire(&byte_19FE188);
    if ( v2 != 0 )
    {
      sub_256FD0(&unk_19FE120, 19246190);
      sub_256FD0(&unk_19FE128, 19246190);
      sub_256FD0(&unk_19FE130, 19246190);
      sub_256FD0(&unk_19FE138, "pc");
      sub_256FD0(&unk_19FE140, 19246190);
      sub_256FD0(&unk_19FE148, "xb1");
      sub_256FD0(&unk_19FE150, 19246190);
      sub_256FD0(&unk_19FE158, "ps4");
      sub_256FD0(&unk_19FE160, "android");
      sub_256FD0(&unk_19FE168, "ios");
      sub_256FD0(&unk_19FE170, "osx");
      sub_256FD0(&unk_19FE178, "tvos");
      sub_256FD0(&unk_19FE180, "nx");
      _cxa_guard_release(&byte_19FE188);
    }
  }
  v3 = (char *)&unk_19FE120 + 8 * a1;
  if ( a1 >= 0xD )
    v3 = (char *)&unk_18EFB28;
  return *(_QWORD *)v3;
}
