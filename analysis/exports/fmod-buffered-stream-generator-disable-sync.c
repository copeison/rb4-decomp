__int64 __fastcall fmod_buffered_stream_generator_disable_sync(__int64 a1)
{
  __int64 v2; // r14

  v2 = a1 + 80;
  (*(void (__fastcall **)(__int64))(*(_QWORD *)(a1 + 80) + 40LL))(a1 + 80);
  *(_BYTE *)(a1 + 525) = 0;
  return (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)(a1 + 80) + 56LL))(v2);
}
