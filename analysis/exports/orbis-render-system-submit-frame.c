// Waits for a submit-done token, consumes it, flushes active command state, submits the primary frame owner, and finalizes collected frame objects.
double __fastcall orbis_render_system_submit_frame(__int64 a1, _QWORD *a2)
{
  __int64 v2; // rax
  __int64 v5; // r14
  unsigned __int64 v6; // rbx
  __int64 savedregs; // [rsp+28h] [rbp+0h]

  savedregs = v2;
  v5 = a1 + 4288;
  scePthreadMutexLock(a1 + 4288);
  ++*(_DWORD *)(a1 + 4280);
  nullsub_39(a1);
  while ( *(_QWORD *)(a1 + 3840) == 0 )
    scePthreadCondWait(a1 + 3832, *(_QWORD *)(a1 + 3824));
  *(_QWORD *)(a1 + 3840) = 0;
  nullsub_40(a1);
  if ( *(_BYTE *)(a1 + 64) != 0 )
    sub_3DEF20(a1);
  sub_8E82D0(*(_QWORD *)(a1 + 56));
  if ( a2[1] != 0 )
  {
    v6 = 0;
    do
      sub_8E2890(*(_QWORD *)(*a2 + 8 * v6++));
    while ( v6 < a2[1] );
  }
  --*(_DWORD *)(a1 + 4280);
  return scePthreadMutexUnlock(v5);
}
