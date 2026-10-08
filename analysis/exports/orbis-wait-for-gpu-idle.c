// Yields until all ten submission counters for the active buffer are zero.
__int64 __fastcall orbis_wait_for_gpu_idle(__int64 a1)
{
  __int64 v2; // rdi
  __int64 v3; // rax

  nullsub_39(a1);
  if ( *(_BYTE *)(a1 + 64) != 0 )
    goto LABEL_4;
  while ( 1 )
  {
    v3 = *(_QWORD *)(a1 + 56);
    if ( *(int *)((char *)&dword_22880[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_22884[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_22888[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_2288C[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_22890[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_22894[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_22898[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_2289C[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_228A0[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0
      && *(int *)((char *)&dword_228A4[10 * *(_QWORD *)(v3 + 265616)] + v3) == 0 )
    {
      break;
    }
    scePthreadYield(v2);
    if ( *(_BYTE *)(a1 + 64) != 0 )
LABEL_4:
      sub_3DEF20(a1);
  }
  return nullsub_40(a1);
}
