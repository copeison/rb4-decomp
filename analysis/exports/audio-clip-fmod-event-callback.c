// Studio event callback that binds HMX DSP user data and records event readiness.
int __fastcall audio_clip_fmod_event_callback(int callback_type, void *event_instance)
{
  _QWORD *v3; // rax
  int i; // ebx
  _QWORD *v7; // rax
  int v8; // r14d
  __int64 v11; // rdi
  __int64 v12; // r15
  _QWORD *v13; // rbx
  _QWORD *v14; // rcx
  _QWORD *v15; // [rsp+8h] [rbp-168h] BYREF
  _BYTE v16[16]; // [rsp+10h] [rbp-160h] BYREF
  FMOD::DSP *v17; // [rsp+20h] [rbp-150h] BYREF
  int v18; // [rsp+2Ch] [rbp-144h] BYREF
  FMOD::ChannelControl *v19; // [rsp+30h] [rbp-140h] BYREF
  _QWORD *v20; // [rsp+38h] [rbp-138h] BYREF
  void *v21[38]; // [rsp+40h] [rbp-130h] BYREF

  v21[32] = (void *)0x6365786562696C2FLL;
  if ( callback_type == 2 || callback_type == 256 )
  {
    FMOD::Studio::EventInstance::getUserData((FMOD::Studio::EventInstance *)event_instance, v21);
    v3 = v21[0];
    if ( v21[0] != nullptr )
LABEL_9:
      *((_BYTE *)v3 + 432) = 1;
  }
  else if ( callback_type == 128 )
  {
    FMOD::Studio::EventInstance::getUserData((FMOD::Studio::EventInstance *)event_instance, (void **)&v20);
    v3 = v20;
    if ( *((_BYTE *)v20 + 432) == 0 )
    {
      if ( *((_DWORD *)v20 + 7) != 6 )
      {
        FMOD::Studio::EventInstance::getChannelGroup(event_instance, &v19);
        v18 = 0;
        FMOD::ChannelControl::getNumDSPs(v19, &v18);
        if ( v18 > 0 )
        {
          for ( i = 0; i < v18; ++i )
          {
            v17 = nullptr;
            FMOD::ChannelControl::getDSP(v19, i, &v17);
            FMOD::DSP::getInfo(v17, (char *)v21, nullptr, nullptr, nullptr, nullptr);
            sub_2550C0(v16, v21);
            v8 = sub_254930(v16, 0, 4, "HMX.");
            sub_255550(v16);
            if ( v8 == 0 )
            {
              v15 = nullptr;
              FMOD::DSP::getUserData(v17, (void **)&v15);
              v7 = v15;
              if ( v15 != nullptr )
              {
                v15[2] = v20;
                if ( v7[3] != 0 )
                  sub_407C0();
              }
            }
          }
        }
        _RAX = v20;
        __asm
        {
          vcvtsi2ss xmm0, xmm0, dword ptr [rcx+0C8h]
          vmovss  dword ptr [rax+1B4h], xmm0
        }
        FMOD::ChannelControl::addDSP(v19, -3, (FMOD::DSP *)_RAX[49]);
        v11 = v20[8];
        if ( v11 != 0 && (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v11 + 128LL))(v11) != 0 )
          FMOD::ChannelGroup::addGroup(0, v19, 1, 0);
        v12 = v20[15];
        v13 = v20 + 10;
        if ( v20 == nullptr )
          v13 = nullptr;
        scePthreadMutexLock(v12 + 80);
        v13[4] = v12 + 88;
        ++*(_QWORD *)(v12 + 104);
        v14 = *(_QWORD **)(v12 + 96);
        v13[3] = v14;
        v13[2] = v12 + 88;
        *v14 = v13 + 2;
        *(_QWORD *)(v12 + 96) = v13 + 2;
        scePthreadMutexUnlock(v12 + 80);
        v3 = v20;
      }
      goto LABEL_9;
    }
  }
  return 0;
}
