__int64 __fastcall fmod_sound_to_pcm_callback_release_consumer(__int64 a1)
{
  __int64 result; // rax

  if ( a1 != 0 )
    return (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 8LL))(a1);
  return result;
}
