// Returns the first active render-target state's width and height, or zero when absent.
unsigned __int64 __fastcall render_frame_owner_output_extent(__int64 a1)
{
  __int64 v1; // rax
  __int64 v2; // rdx
  unsigned __int64 v3; // rcx
  __int64 v4; // rax

  v1 = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 24LL))(a1);
  if ( v2 != 0 )
  {
    v4 = *(_QWORD *)(*(_QWORD *)v1 + 24LL);
    v3 = v4 & 0xFFFFFFFF00000000LL;
    v4 = (unsigned int)v4;
  }
  else
  {
    v4 = 0;
    v3 = 0;
  }
  return v3 | v4;
}
