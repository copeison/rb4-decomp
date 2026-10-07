// Swaps deferred-release buffers, stops channels, and releases DSPs.
void __fastcall fmod_process_deferred_releases(void *observer)
{
  __int64 v1; // r14
  __int64 v2; // r13
  __int64 v3; // r13
  FMOD::ChannelControl **v4; // rbx
  FMOD::ChannelControl **v5; // r12
  FMOD::ChannelControl ***v6; // r15
  FMOD::ChannelControl ***v7; // r13
  FMOD::DSP *v8; // r14

  v1 = *((_QWORD *)observer + 4);
  scePthreadMutexLock(v1 + 712);
  v2 = *(int *)(v1 + 784);
  *(_DWORD *)(v1 + 784) = (*(_DWORD *)(v1 + 784) & 1) == 0;
  scePthreadMutexUnlock(v1 + 712);
  v3 = 32 * v2;
  v4 = *(FMOD::ChannelControl ***)(v1 + v3 + 720);
  v5 = *(FMOD::ChannelControl ***)(v1 + v3 + 728);
  v6 = (FMOD::ChannelControl ***)(v1 + v3 + 728);
  if ( v4 != v5 )
  {
    v7 = (FMOD::ChannelControl ***)(v1 + v3 + 720);
    do
    {
      v8 = v4[1];
      FMOD::ChannelControl::stop(*v4);
      FMOD::DSP::release(v8);
      v4 += 2;
    }
    while ( v4 != v5 );
    v4 = *v7;
  }
  *v6 = v4;
}
