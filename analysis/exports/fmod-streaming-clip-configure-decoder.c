__int64 __fastcall fmod_streaming_clip_configure_decoder(__int64 a1, __int64 a2, int a3)
{
  __int64 (__fastcall *v6)(__int64, __int64, __int64, _QWORD, double); // r14

  (*(void (__fastcall **)(__int64))(*(_QWORD *)a1 + 224LL))(a1);
  *(_QWORD *)(a1 + 16) = a2;
  *(_DWORD *)(a1 + 24) = a3;
  *(_QWORD *)(a1 + 312) = 0;
  v6 = *(__int64 (__fastcall **)(__int64, __int64, __int64, _QWORD, double))(*(_QWORD *)(a1 + 80) + 16LL);
  _XMM0 = audio_get_sample_rate();
  __asm { vcvtsd2ss xmm0, xmm0, xmm0 }
  return v6(a1 + 80, 2, 128, 0, *(double *)&_XMM0);
}
