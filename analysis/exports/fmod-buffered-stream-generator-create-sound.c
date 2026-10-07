__int64 __fastcall fmod_buffered_stream_generator_create_sound(__int64 a1, _QWORD *a2)
{
  __int64 v2; // r15
  __int64 v5; // rax
  int v6; // ecx
  __int64 v7; // rbx
  __int64 v8; // rax
  void *v9; // rdx
  unsigned int v10; // ebx
  int Sound; // eax
  __int64 v12; // rdi

  v5 = *(_QWORD *)(a1 + 72);
  v6 = *(_DWORD *)(v5 + 16);
  if ( (unsigned int)(v6 - 1) <= 1 )
  {
    if ( v6 == 2 )
    {
      if ( *(_QWORD *)(v5 + 760) != 0 )
        v2 = *(_QWORD *)(v5 + 768);
    }
    else if ( *(_QWORD *)(v5 + 280) != 0 )
    {
      v2 = *(_QWORD *)(v5 + 288);
    }
  }
  v7 = *(_QWORD *)(a1 + 64);
  if ( v7 != 0 )
  {
    v8 = sub_5C20(&g_sound_manager);
    v9 = &loc_14090;
    if ( v7 == v8 )
      v9 = &loc_14080;
  }
  else
  {
    v9 = &loc_14080;
  }
  v10 = 0;
  Sound = FMOD::System::createSound(v2, *(_QWORD *)(*a2 + 48LL), v9, 0, a1 + 456);
  *(_BYTE *)(a1 + 480) = 1;
  if ( Sound == 0 )
  {
    if ( *a2 != 0 )
      sub_1ADEB0(*a2);
    v12 = *(_QWORD *)(a1 + 272);
    if ( v12 != 0 )
      sub_1ADEF0(v12);
    LOBYTE(v10) = 1;
    *(_QWORD *)(a1 + 272) = *a2;
  }
  return v10;
}
