// Stops the voice and pumps synchronous Studio updates until teardown completes.
void __fastcall audio_clip_fmod_stop_and_wait(void *clip)
{
  __int64 v3; // rax
  int v4; // ecx
  __int64 v5; // r14
  __int64 v6; // rdi

  _RBX = (char *)clip;
  *((_BYTE *)clip + 444) = 1;
  audio_clip_fmod_defer_channel_release(clip);
  audio_clip_fmod_release_event_instance(_RBX);
  v3 = *((_QWORD *)_RBX + 9);
  v4 = *(_DWORD *)(v3 + 16);
  if ( (unsigned int)(v4 - 1) > 1 )
    goto LABEL_8;
  if ( v4 == 2 )
  {
    v5 = *(_QWORD *)(v3 + 760);
    if ( v5 == 0 || *(_QWORD *)(v3 + 768) == 0 )
      goto LABEL_8;
LABEL_13:
    while ( *((_DWORD *)_RBX + 7) != 5 )
    {
      usleep(1000);
      FMOD::Studio::System::update(v5);
      audio_clip_fmod_defer_channel_release(_RBX);
      audio_clip_fmod_release_event_instance(_RBX);
    }
    goto LABEL_11;
  }
  v5 = *(_QWORD *)(v3 + 280);
  if ( v5 != 0 && *(_QWORD *)(v3 + 288) != 0 )
    goto LABEL_13;
LABEL_8:
  *((_DWORD *)_RBX + 7) = 6;
  v6 = *((_QWORD *)_RBX + 15);
  if ( v6 != 0 )
    sub_57560(v6, _RBX + 80);
  __asm
  {
    vxorps  ymm0, ymm0, ymm0
    vmovups ymmword ptr [rbx+188h], ymm0
  }
  *((_QWORD *)_RBX + 53) = 0;
  *((_DWORD *)_RBX + 7) = 5;
LABEL_11:
  _RBX[444] = 0;
}
