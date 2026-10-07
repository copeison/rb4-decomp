__int64 fmod_async_file_reader_initialize()
{
  _QWORD v1[4]; // [rsp+0h] [rbp-20h] BYREF

  v1[1] = 0x6365786562696C2FLL;
  qword_19F3008 = (__int64)&unk_19F3020;
  scePthreadCondattrInit(v1);
  scePthreadCondInit(&unk_19F3010, v1, "Condition");
  sub_258C60("stream_reader", v1);
  sub_259210(
    (unsigned int)&unk_19F2F78,
    (unsigned int)fmod_async_file_reader_thread,
    0,
    (unsigned int)"FmodFileWrapper",
    *(_QWORD *)(v1[0] + 24LL),
    *(_DWORD *)(v1[0] + 32LL),
    *(_DWORD *)(v1[0] + 16LL),
    *(_QWORD *)(v1[0] + 40LL));
  sub_25C430(&xmmword_19F2F80);
  return 0x6365786562696C2FLL;
}
