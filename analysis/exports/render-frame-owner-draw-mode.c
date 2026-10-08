// Returns the first active render-target state's draw mode, or -1 when absent.
__int64 __fastcall render_frame_owner_draw_mode(__int64 a1)
{
  __int64 v1; // rax
  __int64 v2; // rdx

  v1 = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 24LL))(a1);
  if ( v2 != 0 )
    return *(unsigned int *)(*(_QWORD *)v1 + 16LL);
  else
    return 0xFFFFFFFFLL;
}
