// Creates the custom DSP and starts either a low-level channel or Studio event instance.
void __fastcall audio_clip_fmod_start(void *clip, void *unused, const void *play_options)
{
  __int64 v3; // r12
  __int64 v8; // rax
  int v9; // ecx
  FMOD::ChannelControl **v10; // r13
  _QWORD *v11; // r15
  __int64 v12; // rdi
  __int64 v14; // rdi
  __int64 v15; // rbx
  _QWORD *v16; // rcx
  FMOD::Studio::EventInstance **v17; // r15
  _QWORD *v18; // rax
  _QWORD *v20; // r12
  _QWORD *v22; // [rsp+0h] [rbp-50h]
  __int64 v23; // [rsp+8h] [rbp-48h]
  _BYTE *v24; // [rsp+8h] [rbp-48h]
  bool v25; // [rsp+17h] [rbp-39h] BYREF
  FMOD::Studio::EventDescription *v26[7]; // [rsp+18h] [rbp-38h] BYREF

  _RBX = play_options;
  _R14 = (char *)clip;
  v26[1] = (FMOD::Studio::EventDescription *)0x6365786562696C2FLL;
  sub_E0C70(clip, unused);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  _R14[444] = 0;
  _R14[432] = 0;
  *((_DWORD *)_R14 + 109) = 0;
  __asm { vmovups ymmword ptr [r14+188h], ymm0 }
  v8 = *((_QWORD *)_R14 + 9);
  v9 = *(_DWORD *)(v8 + 16);
  if ( (unsigned int)(v9 - 1) > 1 )
    goto LABEL_5;
  if ( v9 != 2 )
  {
    v23 = *(_QWORD *)(v8 + 280);
    if ( v23 != 0 )
    {
      v3 = *(_QWORD *)(v8 + 288);
      goto LABEL_6;
    }
    goto LABEL_23;
  }
  v23 = *(_QWORD *)(v8 + 760);
  if ( v23 == 0 )
  {
LABEL_23:
    v8 = 0;
LABEL_5:
    v23 = v8;
    goto LABEL_6;
  }
  v3 = *(_QWORD *)(v8 + 768);
LABEL_6:
  FMOD::System::createDSP(v3, &unk_19B4840, _R14 + 392);
  FMOD::DSP::setUserData(*((FMOD::DSP **)_R14 + 49), _R14);
  v26[0] = nullptr;
  if ( *((_DWORD *)_RBX + 10) != 1 )
  {
LABEL_10:
    v10 = (FMOD::ChannelControl **)(_R14 + 400);
    v22 = _R14 + 408;
    FMOD::System::playDSP(v3, *((_QWORD *)_R14 + 49), 0, 1, _R14 + 400);
    if ( *((_DWORD *)_RBX + 10) == 2 )
    {
      v11 = _R14 + 416;
      if ( (unsigned int)FMOD::Studio::System::getBus(v23, *((_QWORD *)_RBX + 6), _R14 + 416) != 0 )
      {
        *v11 = 0;
      }
      else
      {
        FMOD::Studio::Bus::getChannelGroup(*v11, v22);
        FMOD::Channel::setChannelGroup(*v10, *v22);
      }
    }
    v12 = *((_QWORD *)_R14 + 8);
    if ( v12 != 0 )
    {
      if ( (*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)v12 + 200LL))(v12) != 0 )
      {
        FMOD::ChannelControl::setMode(*v10, 0x10u);
        __asm { vmovss  xmm0, dword ptr [rbx+38h] }
        FMOD::ChannelControl::set3DSpread(*v10, *(float *)&_XMM0);
      }
      v14 = *((_QWORD *)_R14 + 8);
      if ( v14 != 0 && (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v14 + 128LL))(v14) != 0 )
      {
        if ( *v22 != 0 )
          FMOD::ChannelGroup::addGroup(0, *v22, 1, 0);
        else
          FMOD::Channel::setChannelGroup(*v10, 0);
      }
    }
    FMOD::Channel::getFrequency(*((FMOD::Channel **)_R14 + 50), (float *)_R14 + 109);
    FMOD::ChannelControl::setPaused(*((FMOD::ChannelControl **)_R14 + 50), *((_BYTE *)_RBX + 16));
    v15 = *((_QWORD *)_R14 + 15);
    scePthreadMutexLock(v15 + 80);
    *((_QWORD *)_R14 + 14) = v15 + 88;
    ++*(_QWORD *)(v15 + 104);
    v16 = *(_QWORD **)(v15 + 96);
    *((_QWORD *)_R14 + 13) = v16;
    *((_QWORD *)_R14 + 12) = v15 + 88;
    *v16 = _R14 + 96;
    *(_QWORD *)(v15 + 96) = _R14 + 96;
    scePthreadMutexUnlock(v15 + 80);
    return;
  }
  if ( (unsigned int)FMOD::Studio::System::getEvent(v23, *((_QWORD *)_RBX + 6), v26) != 0
    || (FMOD::Studio::EventDescription::isOneshot(v26[0], &v25), v25) )
  {
    v26[0] = nullptr;
    goto LABEL_10;
  }
  if ( *((_DWORD *)_RBX + 10) != 1 || v26[0] == nullptr )
    goto LABEL_10;
  v17 = (FMOD::Studio::EventInstance **)(_R14 + 424);
  FMOD::Studio::EventDescription::createInstance(v26[0], _R14 + 424);
  FMOD::Studio::EventInstance::setUserData(*((FMOD::Studio::EventInstance **)_R14 + 53), _R14);
  v24 = _RBX;
  v18 = *((_QWORD **)_RBX + 11);
  if ( v18 != nullptr )
  {
    _RBX = (_QWORD *)*v18;
    v20 = (_QWORD *)v18[1];
    if ( (_QWORD *)*v18 != v20 )
    {
      do
      {
        __asm { vmovss  xmm0, dword ptr [rbx+8] }
        (*(void (__fastcall **)(char *, _QWORD, double))(*(_QWORD *)_R14 + 80LL))(_R14, *_RBX, *(double *)&_XMM0);
        _RBX += 2;
      }
      while ( _RBX != v20 );
    }
  }
  FMOD::Studio::EventInstance::setCallback(*v17, audio_clip_fmod_event_callback, 386);
  FMOD::Studio::EventInstance::setPaused(*v17, v24[16]);
  FMOD::Studio::EventInstance::start(*v17);
}
