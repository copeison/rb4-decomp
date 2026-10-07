__int64 fmod_async_file_reader_shutdown()
{
  double v0; // xmm0_8
  __int64 result; // rax

  scePthreadMutexLock(&unk_19F3020);
  ++dword_19F3018;
  byte_19F3048 = 1;
  scePthreadCondSignal(&unk_19F3010);
  --dword_19F3018;
  v0 = scePthreadMutexUnlock(&unk_19F3020);
  result = sub_25C530(&xmmword_19F2F80, v0);
  if ( qword_19F3008 != 0 )
  {
    result = scePthreadCondDestroy(&unk_19F3010);
    qword_19F3008 = 0;
  }
  return result;
}
