__int64 __fastcall fmod_recording_target_update(__int64 a1)
{
  __int64 v1; // rdi
  __int64 result; // rax

  v1 = a1 + 480;
  if ( *(_BYTE *)(v1 + 696) != 0 )
    return FMOD::Studio::System::update(*(_QWORD *)(v1 + 280));
  return result;
}
