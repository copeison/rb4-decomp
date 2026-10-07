// Constructs the FMOD-backed recording target; embeds FmodAudioState at +0x1E0 and binds the custom output callback.
__int64 __fastcall fmod_recording_target_construct(
        __int64 a1,
        __int64 a2,
        __int64 a3,
        int a4,
        unsigned int a5,
        unsigned int a6,
        __m128 a7,
        unsigned int a8)
{
  int v13; // ecx
  int v14; // r8d
  int v15; // r9d
  __int64 v18; // rdi
  __int64 v19; // r14

  _RBX = a1;
  sub_11289D0(a1, a2, a3, a4, a5, a6, a7, a8);
  *(_QWORD *)_RBX = &vtable_FmodRecordingAudioRenderTarget;
  sub_276530(_RBX + 480);
  __asm { vxorps  xmm0, xmm0, xmm0 }
  __asm { vmovups xmmword ptr [rbx+528h], xmm0 }
  *(_DWORD *)(_RBX + 1336) = 700;
  *(_DWORD *)(_RBX + 1340) = 0;
  *(_QWORD *)(_RBX + 1344) = 0;
  *(double *)&_XMM0 = sub_9290((int)_RBX + 1352, 32, (unsigned int)"Unknown Thread!", v13, v14, v15);
  __asm
  {
    vxorps  xmm0, xmm0, xmm0
    vmovups xmmword ptr [rbx+568h], xmm0
  }
  *(_DWORD *)(_RBX + 1400) = 0;
  *(_QWORD *)(_RBX + 1440) = 0;
  __asm
  {
    vmovups xmmword ptr [rbx+58Ch], xmm0
    vmovups xmmword ptr [rbx+580h], xmm0
  }
  *(_DWORD *)(_RBX + 16) = 2;
  fmod_audio_state_initialize((_DWORD *)(_RBX + 480), a2, 0, 1u, a4, 2, a5, a5, 0);
  *(_QWORD *)(_RBX + 848) = _RBX;
  fmod_audio_configure_speakers((void *)(_RBX + 480), a8);
  fmod_audio_initialize_custom_output(_RBX + 480);
  v18 = *(_QWORD *)(_RBX + 832);
  v19 = _RBX + 800;
  if ( v18 != 0 )
  {
    (*(void (__fastcall **)(__int64, bool))(*(_QWORD *)v18 + 32LL))(v18, v18 != v19);
    *(_QWORD *)(_RBX + 832) = 0;
  }
  *(_QWORD *)(_RBX + 800) = &vtable_FmodRecordingOutputCallback;
  *(_QWORD *)(_RBX + 808) = _RBX;
  *(_QWORD *)(_RBX + 832) = v19;
  return sub_C15E0(&unk_19C90B0, _RBX);
}
