// Detaches and releases a ready Studio event instance and its custom DSP.
void __fastcall audio_clip_fmod_release_event_instance(void *clip)
{
  bool v2; // zf
  __int64 v3; // rdi
  FMOD::DSP *v4; // r12
  int ChannelGroup; // eax
  _QWORD *v7; // r14
  FMOD::DSP *v11; // rdi
  FMOD::ChannelControl *v12[6]; // [rsp+0h] [rbp-30h] BYREF

  _RBX = (FMOD::DSP **)clip;
  v12[1] = (FMOD::ChannelControl *)0x6365786562696C2FLL;
  if ( *((_QWORD *)clip + 53) != 0 )
  {
    v2 = *((_BYTE *)clip + 432) == 0;
    *((_DWORD *)clip + 7) = 6;
    if ( !v2 )
    {
      v3 = *((_QWORD *)clip + 15);
      if ( v3 != 0 && (sub_57560(v3, _RBX + 10), (v4 = _RBX[15]) != nullptr) )
      {
        scePthreadMutexLock((char *)v4 + 40);
        ++*((_DWORD *)v4 + 8);
        FMOD::DSP::setUserData(_RBX[49], nullptr);
        --*((_DWORD *)v4 + 8);
        scePthreadMutexUnlock((char *)v4 + 40);
      }
      else
      {
        FMOD::DSP::setUserData(_RBX[49], nullptr);
      }
      ChannelGroup = FMOD::Studio::EventInstance::getChannelGroup(_RBX[53], v12);
      if ( ChannelGroup != 0 )
      {
        v7 = _RBX + 49;
        if ( ChannelGroup == 30 )
        {
          __asm
          {
            vxorps  xmm0, xmm0, xmm0
            vmovups xmmword ptr [rbx+198h], xmm0
          }
          _RBX[53] = nullptr;
LABEL_13:
          *v7 = 0;
          *((_DWORD *)_RBX + 7) = 5;
          return;
        }
      }
      else
      {
        v7 = _RBX + 49;
        FMOD::ChannelControl::removeDSP(v12[0], _RBX[49]);
      }
      FMOD::Studio::EventInstance::stop(_RBX[53], 1);
      *(double *)&_XMM0 = FMOD::Studio::EventInstance::release(_RBX[53]);
      __asm
      {
        vxorps  xmm0, xmm0, xmm0
        vmovups xmmword ptr [rbx+198h], xmm0
      }
      _RBX[53] = nullptr;
      v11 = _RBX[49];
      if ( v11 != nullptr )
        FMOD::DSP::release(v11);
      goto LABEL_13;
    }
  }
}
