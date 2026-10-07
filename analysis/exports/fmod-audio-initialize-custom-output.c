__int64 __fastcall fmod_audio_initialize_custom_output(__int64 a1)
{
  _QWORD *v2; // r14
  int v3; // r15d
  __int64 result; // rax
  _BYTE v5[4]; // [rsp+10h] [rbp-40h] BYREF
  int v6; // [rsp+14h] [rbp-3Ch] BYREF
  _BYTE v7[4]; // [rsp+18h] [rbp-38h] BYREF
  _BYTE v8[4]; // [rsp+1Ch] [rbp-34h] BYREF
  _BYTE v9[4]; // [rsp+20h] [rbp-30h] BYREF
  unsigned int v10; // [rsp+24h] [rbp-2Ch] BYREF
  __int64 v11; // [rsp+28h] [rbp-28h]

  v11 = 0x6365786562696C2FLL;
  FMOD::Studio::System::create(a1 + 280, 69636);
  v2 = (_QWORD *)(a1 + 288);
  FMOD::Studio::System::getLowLevelSystem(*(_QWORD *)(a1 + 280), a1 + 288);
  FMOD::Studio::System::setUserData(*(_QWORD *)(a1 + 280), a1);
  FMOD::System::setUserData(*(_QWORD *)(a1 + 288), a1);
  FMOD::System::registerOutput(*(_QWORD *)(a1 + 288), &g_fmod_buffered_output_description, &v10);
  FMOD::System::setOutputByPlugin(*(_QWORD *)(a1 + 288), v10);
  FMOD::System::setDSPBufferSize(*(_QWORD *)(a1 + 288), *(unsigned int *)(a1 + 204), *(unsigned int *)(a1 + 208));
  FMOD::System::setSoftwareChannels(*(_QWORD *)(a1 + 288), *(unsigned int *)(a1 + 304));
  FMOD::System::setSoftwareFormat(
    *(_QWORD *)(a1 + 288),
    *(unsigned int *)(a1 + 200),
    *(unsigned int *)(a1 + 300),
    *(unsigned int *)(a1 + 216));
  v3 = FMOD::Studio::System::initialize(*(_QWORD *)(a1 + 280), *(unsigned int *)(a1 + 304), 4, 7, a1);
  FMOD::System::getSoftwareFormat(*(_QWORD *)(a1 + 288), v7, v9, v8);
  FMOD::System::getDSPBufferSize(*(_QWORD *)(a1 + 288), &v6, v5);
  *(_DWORD *)(a1 + 296) = v6;
  fmod_register_custom_dsp_plugins(a1);
  result = 1;
  if ( v3 == 0 )
  {
    FMOD::System::setFileSystem(
      *v2,
      fmod_file_open,
      fmod_file_close,
      fmod_file_read,
      fmod_file_seek,
      fmod_file_async_read,
      fmod_file_async_cancel,
      -1);
    FMOD::System::setCallback(*v2, fmod_system_callback, 96);
    return 0;
  }
  return result;
}
