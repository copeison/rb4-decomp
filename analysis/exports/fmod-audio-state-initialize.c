__int64 __fastcall fmod_audio_state_initialize(
        _DWORD *a1,
        __int64 a2,
        __int64 a3,
        unsigned int a4,
        int a5,
        int a6,
        unsigned int a7,
        int a8,
        char a9)
{
  unsigned int v13; // r15d

  sub_11281D0(a1);
  v13 = 0;
  sem_init(a1 + 170, 0, 1);
  a1[51] = a5;
  a1[52] = a6;
  a1[76] = a8;
  sub_276FD0((__int64)a1, a6);
  if ( a9 != 0 )
    v13 = fmod_audio_initialize((__int64)a1, 0, a4, a7);
  (*(void (__fastcall **)(_DWORD *, _DWORD *))(*(_QWORD *)a1 + 104LL))(a1, a1 + 198);
  return v13;
}
