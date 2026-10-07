__int64 __fastcall fmod_studio_sound_generator_event_callback(int a1, FMOD::Studio::EventInstance *a2)
{
  _QWORD **v3; // rax
  int i; // ebx
  _QWORD *v5; // rax
  __int64 v6; // rdi
  int v7; // r14d
  _QWORD *v9; // [rsp+8h] [rbp-168h] BYREF
  _BYTE v10[16]; // [rsp+10h] [rbp-160h] BYREF
  FMOD::DSP *v11; // [rsp+20h] [rbp-150h] BYREF
  int v12; // [rsp+2Ch] [rbp-144h] BYREF
  FMOD::ChannelControl *v13; // [rsp+30h] [rbp-140h] BYREF
  _QWORD **v14; // [rsp+38h] [rbp-138h] BYREF
  void *v15[38]; // [rsp+40h] [rbp-130h] BYREF

  v15[32] = (void *)0x6365786562696C2FLL;
  if ( a1 == 2 || a1 == 32 )
  {
    FMOD::Studio::EventInstance::getUserData(a2, v15);
    v3 = (_QWORD **)v15[0];
    if ( v15[0] != nullptr )
LABEL_14:
      *((_BYTE *)v3 + 204) = 1;
  }
  else if ( a1 == 8 )
  {
    FMOD::Studio::EventInstance::getUserData(a2, (void **)&v14);
    v3 = v14;
    if ( (unsigned int)(*((_DWORD *)v14 + 7) - 5) >= 2 )
    {
      FMOD::Studio::EventInstance::getChannelGroup(a2, &v13);
      v12 = 0;
      FMOD::ChannelControl::getNumDSPs(v13, &v12);
      if ( v12 > 0 )
      {
        for ( i = 0; i < v12; ++i )
        {
          v11 = nullptr;
          FMOD::ChannelControl::getDSP(v13, i, &v11);
          FMOD::DSP::getInfo(v11, (char *)v15, nullptr, nullptr, nullptr, nullptr);
          sub_2550C0(v10, v15);
          v7 = sub_254930(v10, 0, 4, "HMX.");
          sub_255550(v10);
          if ( v7 == 0 )
          {
            v9 = nullptr;
            FMOD::DSP::getUserData(v11, (void **)&v9);
            v5 = v9;
            if ( v9 != nullptr )
            {
              v6 = (__int64)v14;
              v9[2] = v14;
              if ( v5[3] != 0 )
                audio_clip_notify_event_dsp_attached(v6);
            }
          }
        }
      }
      (*(void (__fastcall **)(_QWORD *))(*v14[8] + 128LL))(v14[8]);
      v3 = v14;
    }
    goto LABEL_14;
  }
  return 0;
}
