__int64 __fastcall fmod_dialog_programmer_sound_callback(int a1, FMOD::Studio::EventInstance *a2, FMOD::Sound **a3)
{
  unsigned int v5; // r15d
  __int64 v7; // rax
  __int64 v8; // rbx
  unsigned int SoundInfo; // eax
  unsigned int v13; // eax
  FMOD::Sound *v14; // [rsp+8h] [rbp-138h] BYREF
  __int64 v15; // [rsp+10h] [rbp-130h] BYREF
  unsigned int v16; // [rsp+18h] [rbp-128h]
  _BYTE v17[232]; // [rsp+20h] [rbp-120h] BYREF
  int v18; // [rsp+108h] [rbp-38h]
  void *v19[6]; // [rsp+110h] [rbp-30h] BYREF

  v19[1] = (void *)0x6365786562696C2FLL;
  v19[0] = nullptr;
  FMOD::Studio::EventInstance::getUserData(a2, v19);
  if ( (*(unsigned int (__fastcall **)(void *))(*(_QWORD *)v19[0] + 24LL))(v19[0]) == 5
    || (*(unsigned int (__fastcall **)(void *))(*(_QWORD *)v19[0] + 24LL))(v19[0]) == 6 )
  {
    v5 = 0;
    *((_BYTE *)v19[0] + 340) = 1;
  }
  else
  {
    v5 = 0;
    if ( a1 > 255 )
    {
      if ( a1 == 256 )
      {
        return (unsigned int)FMOD::Sound::release(a3[1]);
      }
      else if ( a1 == 0x2000 && a3 != nullptr )
      {
        LODWORD(v15) = 0;
        *(double *)&_XMM0 = FMOD::Sound::getLength((FMOD::Sound *)a3, (unsigned int *)&v15, 1u);
        _RCX = v19[0];
        __asm
        {
          vcvtsi2ss xmm0, xmm0, rax
          vmovss  dword ptr [rcx+198h], xmm0
        }
      }
    }
    else if ( a1 == 8 )
    {
      (*(void (__fastcall **)(_QWORD))(**((_QWORD **)v19[0] + 25) + 128LL))(*((_QWORD *)v19[0] + 25));
    }
    else if ( a1 == 128 )
    {
      v7 = *((_QWORD *)v19[0] + 26);
      v8 = *(_QWORD *)(v7 + 288);
      SoundInfo = FMOD::Studio::System::getSoundInfo(*(_QWORD *)(v7 + 280), *a3, &v15);
      if ( SoundInfo != 0 )
      {
        return SoundInfo;
      }
      else
      {
        v14 = nullptr;
        v16 |= 0x14200u;
        v13 = FMOD::System::createSound(v8, v15, v16, v17, &v14);
        if ( v13 != 0 )
        {
          return v13;
        }
        else
        {
          a3[1] = v14;
          *((_DWORD *)a3 + 4) = v18;
          sub_11273D0(v19[0], *a3);
        }
      }
    }
  }
  return v5;
}
