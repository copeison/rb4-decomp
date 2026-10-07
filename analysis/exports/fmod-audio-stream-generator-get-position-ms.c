__int128 __usercall fmod_audio_stream_generator_get_position_ms@<xmm0>(__int64 a1@<rdi>)
{
  unsigned __int64 v1; // rdx
  unsigned __int64 v2; // rax
  __int128 result; // xmm0

  if ( *(int *)(a1 + 128) <= 0 )
  {
    v2 = *(_QWORD *)(a1 + 120);
  }
  else
  {
    v1 = __rdtsc();
    v2 = *(_QWORD *)(a1 + 120) + v1 - *(_QWORD *)(a1 + 112);
    *(_QWORD *)(a1 + 120) = v2;
    *(_QWORD *)(a1 + 112) = v1;
  }
  *(double *)&_XMM0 = performance_counter_ticks_to_milliseconds(v2);
  __asm { vcvtsd2ss xmm0, xmm0, xmm0 }
  return result;
}
