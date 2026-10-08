// Stops the submit thread, destroys video synchronization and frame runtime state, removes event 64, deletes the queue, and closes video output.
__int64 __fastcall orbis_render_system_shutdown(__int64 a1, double a2)
{
  *(_BYTE *)(a1 + 3848) = 0;
  sub_25C530((__int64 *)(a1 + 4152));
  if ( *(_QWORD *)(a1 + 3824) != 0 )
  {
    scePthreadCondDestroy(a1 + 3832);
    *(_QWORD *)(a1 + 3824) = 0;
  }
  sub_3DED80(a1);
  sub_3DEEA0((_QWORD *)a1);
  j_sceGnmDeleteEqEvent(*(_QWORD *)(a1 + 3808), 64);
  sceKernelDeleteEqueue(*(_QWORD *)(a1 + 3808));
  return sceVideoOutClose(*(unsigned int *)(a1 + 3804));
}
