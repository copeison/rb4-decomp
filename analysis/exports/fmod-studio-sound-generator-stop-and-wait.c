void __fastcall fmod_studio_sound_generator_stop_and_wait(__int64 a1)
{
  __int64 v2; // rdi

  v2 = *(_QWORD *)(a1 + 192);
  if ( v2 != 0 )
  {
    *(_DWORD *)(a1 + 28) = 6;
    FMOD::Studio::EventInstance::setCallback(v2, 0, 0xFFFFFFFFLL);
    FMOD::Studio::EventInstance::stop(*(_QWORD *)(a1 + 192), 1);
    while ( *(_BYTE *)(a1 + 204) == 0 )
    {
      if ( (*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)a1 + 176LL))(a1) != 0 )
        FMOD::Studio::System::flushCommands(*(FMOD::Studio::System **)(unk_19F29D8 + 280LL));
    }
    *(_DWORD *)(a1 + 28) = 5;
    FMOD::Studio::EventInstance::release(*(FMOD::Studio::EventInstance **)(a1 + 192));
    *(_QWORD *)(a1 + 192) = 0;
    *(_DWORD *)(a1 + 200) = 0;
  }
}
