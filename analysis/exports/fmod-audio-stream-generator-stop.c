__int64 __fastcall fmod_audio_stream_generator_stop(__int64 a1)
{
  __int64 result; // rax

  if ( *(_QWORD *)(a1 + 80) != 0 )
  {
    (*(void (__fastcall **)(__int64))(*(_QWORD *)a1 + 16LL))(a1);
    result = FMOD::Sound::release(*(FMOD::Sound **)(a1 + 80));
    *(_QWORD *)(a1 + 80) = 0;
  }
  *(_DWORD *)(a1 + 28) = 5;
  return result;
}
