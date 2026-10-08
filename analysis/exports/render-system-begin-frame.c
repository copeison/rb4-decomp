// Begins a render frame and prepares the active render context.
__int64 __fastcall render_system_begin_frame(_QWORD *a1)
{
  unsigned int v1; // r14d

  render_system_prepare_frame((__int64)a1, 0);
  LOBYTE(v1) = 1;
  if ( render_system_attach_frame_owner(a1, a1[14]) == 0 )
  {
    v1 = 0;
    render_system_finish_frame((__int64)a1, 0);
  }
  return v1;
}
