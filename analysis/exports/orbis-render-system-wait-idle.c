// Waits for GPU submissions, retires allocations at least two frames old, flushes an active frame, and waits for the primary frame owner.
void __fastcall orbis_render_system_wait_idle(__int64 a1)
{
  __int64 v1; // rax
  double v3; // xmm0_8
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  orbis_wait_for_gpu_idle(a1);
  orbis_release_retired_allocations(a1, v3);
  if ( *(_BYTE *)(a1 + 64) != 0 )
    sub_3DEF20(a1);
  JUMPOUT(0x8E8450);
}
