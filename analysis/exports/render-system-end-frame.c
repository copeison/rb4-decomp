// Ends the active render frame and clears its transient object list.
__int64 __fastcall render_system_end_frame(_QWORD *a1)
{
  scePthreadSelf(a1);
  a1[17] = a1[16];
  a1[15] = 0;
  return render_system_finish_frame((__int64)a1, 0);
}
