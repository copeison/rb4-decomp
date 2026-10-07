__int64 __fastcall fmod_audio_stream_generator_set_position_ms(__int64 a1, __m128 _XMM0)
{
  __int64 result; // rax

  __asm { vcvttss2si rax, xmm0 }
  *(_DWORD *)(a1 + 144) = result;
  return result;
}
