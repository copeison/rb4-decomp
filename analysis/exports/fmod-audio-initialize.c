__int64 __fastcall fmod_audio_initialize(__int64 a1, __int64 a2, unsigned int a3, unsigned int a4)
{
  int v7; // edx
  int v8; // ecx
  int v9; // r8d
  __int64 v10; // rax
  __int64 v11; // rax
  _BYTE *v12; // r15
  _BYTE *v13; // r13
  unsigned __int64 v14; // rax
  __int64 v15; // r14
  unsigned int v19; // r15d
  _QWORD *v24; // [rsp+28h] [rbp-200h]
  _BYTE v25[4]; // [rsp+34h] [rbp-1F4h] BYREF
  int v26; // [rsp+38h] [rbp-1F0h] BYREF
  _BYTE v27[4]; // [rsp+3Ch] [rbp-1ECh] BYREF
  _BYTE v28[4]; // [rsp+40h] [rbp-1E8h] BYREF
  unsigned int v29; // [rsp+44h] [rbp-1E4h] BYREF
  int v30; // [rsp+48h] [rbp-1E0h] BYREF
  __int128 v31; // [rsp+4Ch] [rbp-1DCh]
  __m256 v32; // [rsp+60h] [rbp-1C8h] BYREF
  _BYTE v34[56]; // [rsp+A0h] [rbp-188h]
  _QWORD v35[2]; // [rsp+E0h] [rbp-148h] BYREF
  int v36; // [rsp+F0h] [rbp-138h]
  _BYTE v37[236]; // [rsp+F4h] [rbp-134h] BYREF
  __int64 v38; // [rsp+1E0h] [rbp-48h]

  v38 = 0x6365786562696C2FLL;
  v24 = (_QWORD *)(a1 + 280);
  if ( (unsigned int)FMOD::Studio::System::create(a1 + 280, 69636) == 20 )
  {
    sub_2472C0(v35, "header version is %u.%u.%u", v7, v8, v9);
    v10 = sub_247780(v35, 1);
    v11 = sub_247780(v10, 10);
    sub_247780(v11, 4);
    v12 = (_BYTE *)sub_2484E0(v35);
    sub_247510(v35);
    v35[0] = &unk_18F0FD8;
    sub_258550(v35);
    v13 = v37;
    v35[1] = v37;
    v36 = 128;
    v37[0] = 0;
    v35[0] = &unk_18F0FD8;
    if ( v12 != nullptr && *v12 != 0 )
    {
      v14 = strlen(v12);
      v15 = 128;
      if ( v14 < 0x80 )
        v15 = v14;
      memmove(v37, v12, v15);
      v13 = &v37[v15];
    }
    *v13 = 0;
  }
  FMOD::Studio::System::setUserData(*(_QWORD *)(a1 + 280), a1);
  FMOD::Studio::System::getLowLevelSystem(*(_QWORD *)(a1 + 280), a1 + 288);
  FMOD::System::setUserData(*(_QWORD *)(a1 + 288), a1);
  if ( (unsigned int)FMOD::System::setOutput(*(_QWORD *)(a1 + 288), a3) != 0 )
    FMOD::System::setOutput(*(_QWORD *)(a1 + 288), 0);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  __asm
  {
    vmovups [rsp+228h+var_170], ymm0
    vmovups ymmword ptr [rsp+0A0h], ymm0
    vmovups [rsp+228h+var_1A8], ymm0
    vmovups [rsp+228h+var_1C8], ymm0
  }
  LODWORD(v32.m256_f32[0]) = 120;
  FMOD::System::getAdvancedSettings(*(_QWORD *)(a1 + 288), &v32);
  *(_DWORD *)&v34[40] += 0x4000;
  *(_DWORD *)&v34[48] *= 4;
  *(double *)&_XMM0 = FMOD::System::setAdvancedSettings(*(_QWORD *)(a1 + 288), &v32);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups [rsp+228h+var_1DC], xmm0 }
  v30 = 20;
  FMOD::Studio::System::getAdvancedSettings(*(_QWORD *)(a1 + 280), &v30);
  LODWORD(v31) = 4 * v31;
  FMOD::Studio::System::setAdvancedSettings(*(_QWORD *)(a1 + 280), &v30);
  FMOD::System::setSoftwareChannels(*(_QWORD *)(a1 + 288), a4);
  v19 = FMOD::Studio::System::initialize(*(_QWORD *)(a1 + 280), *(unsigned int *)(a1 + 304), 0, 0, 0);
  if ( v19 == 0 )
  {
    FMOD::System::setFileSystem(
      *(_QWORD *)(a1 + 288),
      fmod_file_open,
      fmod_file_close,
      fmod_file_read,
      fmod_file_seek,
      fmod_file_async_read,
      fmod_file_async_cancel,
      -1);
    FMOD::System::getDriver(*(_QWORD *)(a1 + 288), &v29);
    FMOD::System::getDriverInfo(*(_QWORD *)(a1 + 288), v29, v35, 256, 0, a1 + 200, 0, 0);
    FMOD::System::getSoftwareFormat(*(_QWORD *)(a1 + 288), a1 + 200, v28, v27);
    FMOD::System::getDSPBufferSize(*(_QWORD *)(a1 + 288), &v26, v25);
    *(_DWORD *)(a1 + 296) = v26;
    __asm { vcvtsi2sd xmm0, xmm1, dword ptr [rbx+0C8h]; sample_rate }
    audio_set_mix_format(*(double *)&_XMM0);
    audio_output_dispatcher_set_sample_rate(a1 + 24, *(unsigned int *)(a1 + 200));
    fmod_register_custom_dsp_plugins(a1);
    FMOD::System::setCallback(*(_QWORD *)(a1 + 288), fmod_system_callback, 96);
  }
  FMOD::Studio::System::isValid(*v24);
  FMOD::Studio::System::update(*v24);
  return v19;
}
