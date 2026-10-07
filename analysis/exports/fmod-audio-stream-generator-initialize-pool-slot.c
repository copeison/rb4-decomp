__int64 __fastcall fmod_audio_stream_generator_initialize_pool_slot(__int64 a1, __int64 a2, int a3)
{
  __int64 result; // rax

  result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 224LL))(a1);
  *(_QWORD *)(a1 + 16) = a2;
  *(_DWORD *)(a1 + 24) = a3;
  return result;
}
