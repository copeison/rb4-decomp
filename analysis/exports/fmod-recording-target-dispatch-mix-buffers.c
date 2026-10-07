// Locks the target and embedded audio state around the mix-buffer dispatch.
__int64 __fastcall fmod_recording_target_dispatch_mix_buffers(_QWORD *a1, __int64 a2)
{
  (*(void (__fastcall **)(_QWORD *))(*a1 + 80LL))(a1);
  (*(void (__fastcall **)(_QWORD *, __int64))(a1[60] + 120LL))(a1 + 60, a2);
  return (*(__int64 (__fastcall **)(_QWORD *))(*a1 + 88LL))(a1);
}
