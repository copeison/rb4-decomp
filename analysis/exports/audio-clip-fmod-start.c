// Initializes the AudioClip base state, creates the custom DSP, and starts either a low-level channel or a Studio event.
void __fastcall audio_clip_fmod_start(void *clip, void *sample_source, const void *play_options, void *generator)
{
  __int64 v4; // r12
  __int64 v9; // rax
  int v10; // ecx
  FMOD::ChannelControl **v11; // r13
  _QWORD *v12; // r15
  __int64 v13; // rdi
  __int64 v15; // rdi
  __int64 v16; // rbx
  _QWORD *v17; // rcx
  FMOD::Studio::EventInstance **v18; // r15
  _QWORD *v19; // rax
  _QWORD *v21; // r12
  _QWORD *v23; // [rsp+0h] [rbp-50h]
  __int64 v24; // [rsp+8h] [rbp-48h]
  _BYTE *v25; // [rsp+8h] [rbp-48h]
  bool v26; // [rsp+17h] [rbp-39h] BYREF
  FMOD::Studio::EventDescription *v27[7]; // [rsp+18h] [rbp-38h] BYREF

  _RBX = play_options;
  _R14 = (char *)clip;
  v27[1] = (FMOD::Studio::EventDescription *)0x6365786562696C2FLL;
  sub_E0C70((__int64)clip, (__int64)sample_source, (__int64)play_options, (__int64)generator);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  _R14[444] = 0;
  _R14[432] = 0;
  *((_DWORD *)_R14 + 109) = 0;
  __asm { vmovups ymmword ptr [r14+188h], ymm0 }
  v9 = *((_QWORD *)_R14 + 9);
  v10 = *(_DWORD *)(v9 + 16);
  if ( (unsigned int)(v10 - 1) > 1 )
    goto LABEL_5;
  if ( v10 != 2 )
  {
    v24 = *(_QWORD *)(v9 + 280);
    if ( v24 != 0 )
    {
      v4 = *(_QWORD *)(v9 + 288);
      goto LABEL_6;
    }
    goto LABEL_23;
  }
  v24 = *(_QWORD *)(v9 + 760);
  if ( v24 == 0 )
  {
LABEL_23:
    v9 = 0;
LABEL_5:
    v24 = v9;
    goto LABEL_6;
  }
  v4 = *(_QWORD *)(v9 + 768);
LABEL_6:
  FMOD::System::createDSP(v4, &unk_19B4840, _R14 + 392);
  FMOD::DSP::setUserData(*((FMOD::DSP **)_R14 + 49), _R14);
  v27[0] = nullptr;
  if ( *((_DWORD *)_RBX + 10) != 1 )
  {
LABEL_10:
    v11 = (FMOD::ChannelControl **)(_R14 + 400);
    v23 = _R14 + 408;
    FMOD::System::playDSP(v4, *((_QWORD *)_R14 + 49), 0, 1, _R14 + 400);
    if ( *((_DWORD *)_RBX + 10) == 2 )
    {
      v12 = _R14 + 416;
      if ( (unsigned int)FMOD::Studio::System::getBus(v24, *((_QWORD *)_RBX + 6), _R14 + 416) != 0 )
      {
        *v12 = 0;
      }
      else
      {
        FMOD::Studio::Bus::getChannelGroup(*v12, v23);
        FMOD::Channel::setChannelGroup(*v11, *v23);
      }
    }
    v13 = *((_QWORD *)_R14 + 8);
    if ( v13 != 0 )
    {
      if ( (*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)v13 + 200LL))(v13) != 0 )
      {
        FMOD::ChannelControl::setMode(*v11, 0x10u);
        __asm { vmovss  xmm0, dword ptr [rbx+38h] }
        FMOD::ChannelControl::set3DSpread(*v11, *(float *)&_XMM0);
      }
      v15 = *((_QWORD *)_R14 + 8);
      if ( v15 != 0 && (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v15 + 128LL))(v15) != 0 )
      {
        if ( *v23 != 0 )
          FMOD::ChannelGroup::addGroup(0, *v23, 1, 0);
        else
          FMOD::Channel::setChannelGroup(*v11, 0);
      }
    }
    FMOD::Channel::getFrequency(*((FMOD::Channel **)_R14 + 50), (float *)_R14 + 109);
    FMOD::ChannelControl::setPaused(*((FMOD::ChannelControl **)_R14 + 50), *((_BYTE *)_RBX + 16));
    v16 = *((_QWORD *)_R14 + 15);
    scePthreadMutexLock(v16 + 80);
    *((_QWORD *)_R14 + 14) = v16 + 88;
    ++*(_QWORD *)(v16 + 104);
    v17 = *(_QWORD **)(v16 + 96);
    *((_QWORD *)_R14 + 13) = v17;
    *((_QWORD *)_R14 + 12) = v16 + 88;
    *v17 = _R14 + 96;
    *(_QWORD *)(v16 + 96) = _R14 + 96;
    scePthreadMutexUnlock(v16 + 80);
    return;
  }
  if ( (unsigned int)FMOD::Studio::System::getEvent(v24, *((_QWORD *)_RBX + 6), v27) != 0
    || (FMOD::Studio::EventDescription::isOneshot(v27[0], &v26), v26) )
  {
    v27[0] = nullptr;
    goto LABEL_10;
  }
  if ( *((_DWORD *)_RBX + 10) != 1 || v27[0] == nullptr )
    goto LABEL_10;
  v18 = (FMOD::Studio::EventInstance **)(_R14 + 424);
  FMOD::Studio::EventDescription::createInstance(v27[0], _R14 + 424);
  FMOD::Studio::EventInstance::setUserData(*((FMOD::Studio::EventInstance **)_R14 + 53), _R14);
  v25 = _RBX;
  v19 = *((_QWORD **)_RBX + 11);
  if ( v19 != nullptr )
  {
    _RBX = (_QWORD *)*v19;
    v21 = (_QWORD *)v19[1];
    if ( (_QWORD *)*v19 != v21 )
    {
      do
      {
        __asm { vmovss  xmm0, dword ptr [rbx+8] }
        (*(void (__fastcall **)(char *, _QWORD, double))(*(_QWORD *)_R14 + 80LL))(_R14, *_RBX, *(double *)&_XMM0);
        _RBX += 2;
      }
      while ( _RBX != v21 );
    }
  }
  FMOD::Studio::EventInstance::setCallback(*v18, audio_clip_fmod_event_callback, 386);
  FMOD::Studio::EventInstance::setPaused(*v18, v25[16]);
  FMOD::Studio::EventInstance::start(*v18);
}
