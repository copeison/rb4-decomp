// Returns the interned name for one of the seven render APIs.
__int64 __fastcall render_api_name(unsigned int a1)
{
  int v2; // eax
  char *v3; // rax

  if ( byte_19E4648[0] == 0 )
  {
    _cxa_guard_acquire(byte_19E4648);
    if ( v2 != 0 )
    {
      sub_256FD0(&unk_19E4610, "null");
      sub_256FD0(&unk_19E4618, "dx11");
      sub_256FD0(&unk_19E4620, "ps4");
      sub_256FD0(&unk_19E4628, "mtl");
      sub_256FD0(&unk_19E4630, "vlk");
      sub_256FD0(&unk_19E4638, "nx");
      sub_256FD0(&unk_19E4640, "gles3");
      _cxa_guard_release(byte_19E4648);
    }
  }
  v3 = (char *)&unk_19E4610 + 8 * a1;
  if ( a1 >= 7 )
    v3 = (char *)&unk_18EFB28;
  return *(_QWORD *)v3;
}
