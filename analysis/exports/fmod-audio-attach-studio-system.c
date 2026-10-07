// Attaches or detaches an externally owned FMOD Studio system.
void __fastcall fmod_audio_attach_studio_system(void *state, void *studio_system)
{
  __int64 v4; // rdi
  _BYTE v7[4]; // [rsp+Ch] [rbp-154h] BYREF
  int v8; // [rsp+10h] [rbp-150h] BYREF
  _BYTE v9[4]; // [rsp+14h] [rbp-14Ch] BYREF
  _BYTE v10[4]; // [rsp+18h] [rbp-148h] BYREF
  unsigned int v11; // [rsp+1Ch] [rbp-144h] BYREF
  _BYTE v12[12]; // [rsp+20h] [rbp-140h] BYREF
  _BYTE v13[4]; // [rsp+2Ch] [rbp-134h] BYREF
  _BYTE v14[256]; // [rsp+30h] [rbp-130h] BYREF
  __int64 v15; // [rsp+130h] [rbp-30h]

  v15 = 0x6365786562696C2FLL;
  if ( *((void **)state + 35) != studio_system )
  {
    if ( studio_system != nullptr )
    {
      *((_QWORD *)state + 35) = studio_system;
      FMOD::Studio::System::getLowLevelSystem(studio_system, (char *)state + 288);
      FMOD::System::getVersion(*((_QWORD *)state + 36), v13);
      FMOD::Studio::System::getUserData(*((_QWORD *)state + 35), v12);
      FMOD::Studio::System::setUserData(*((_QWORD *)state + 35), state);
      FMOD::System::getUserData(*((_QWORD *)state + 36), v12);
      FMOD::System::setUserData(*((_QWORD *)state + 36), state);
      FMOD::System::getDriver(*((_QWORD *)state + 36), &v11);
      FMOD::System::getDriverInfo(*((_QWORD *)state + 36), v11, v14, 256, 0, (char *)state + 200, 0, 0);
      FMOD::System::getSoftwareFormat(*((_QWORD *)state + 36), (char *)state + 200, v10, v9);
      FMOD::System::getDSPBufferSize(*((_QWORD *)state + 36), &v8, v7);
      *((_DWORD *)state + 74) = v8;
      audio_output_dispatcher_set_sample_rate((char *)state + 24, *((_DWORD *)state + 50));
      fmod_register_custom_dsp_plugins((__int64)state);
      *((_BYTE *)state + 308) = 0;
      sem_post((char *)state + 680);
      FMOD::System::setCallback(*((_QWORD *)state + 36), fmod_system_callback, 96);
      FMOD::Studio::System::isValid(*((_QWORD *)state + 35));
      FMOD::Studio::System::update(*((_QWORD *)state + 35));
    }
    else
    {
      _R13 = (char *)state + 280;
      while ( (unsigned int)sem_wait((char *)state + 680) != 0 )
        _error(v4);
      *((_BYTE *)state + 308) = 1;
      scePthreadMutexLock((char *)state + 712);
      *((_QWORD *)state + 91) = *((_QWORD *)state + 90);
      *((_QWORD *)state + 95) = *((_QWORD *)state + 94);
      *(double *)&_XMM0 = scePthreadMutexUnlock((char *)state + 712);
      __asm { vxorps  xmm0, xmm0, xmm0 }
      __asm { vmovups xmmword ptr [r13+0], xmm0 }
      sem_post((char *)state + 680);
    }
  }
}
