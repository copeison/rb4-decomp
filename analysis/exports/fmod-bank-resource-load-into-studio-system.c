__int64 __fastcall fmod_bank_resource_load_into_studio_system(_QWORD *a1, __int64 a2, FMOD::Studio::System *a3)
{
  double v5; // xmm0_8
  int SampleLoadingState; // ebx
  __int64 i; // rbx
  FMOD::Studio::Bus *v8; // rdi
  FMOD::Studio::Bank *v9; // rbx
  FMOD::Studio::Bus *v10; // rax
  FMOD::Studio::Bus *v11; // rsi
  FMOD::Studio::Bus *v12; // rdx
  int v14; // [rsp+4h] [rbp-404Ch] BYREF
  FMOD::Studio::Bank *v15; // [rsp+8h] [rbp-4048h] BYREF
  FMOD::Studio::System *v16; // [rsp+10h] [rbp-4040h] BYREF
  int v17; // [rsp+1Ch] [rbp-4034h] BYREF
  FMOD::Studio::Bus *v18[2054]; // [rsp+20h] [rbp-4030h] BYREF

  v18[2048] = (FMOD::Studio::Bus *)0x6365786562696C2FLL;
  v16 = a3;
  if ( (unsigned int)FMOD::Studio::System::loadBankFile(a3, *(const char **)(a2 + 8), 0, &v15) != 0 )
  {
    fmod_bank_resource_unload_all(a1, v5);
  }
  else
  {
    FMOD::Studio::Bank::loadSampleData(v15);
    do
    {
      scePthreadYield();
      SampleLoadingState = FMOD::Studio::Bank::getSampleLoadingState(v15, &v14);
      FMOD::Studio::System::update(a3);
    }
    while ( SampleLoadingState == 0 && v14 == 2 );
    v17 = 0;
    if ( (unsigned int)FMOD::Studio::Bank::getBusList(v15, v18, 2048, &v17) == 0 && v17 > 0 )
    {
      for ( i = 0; i < v17; FMOD::Studio::Bus::lockChannelGroup(v18[i++]) )
        ;
      FMOD::Studio::System::flushCommands(a3);
    }
    v8 = (FMOD::Studio::Bus *)a1[9];
    v9 = v15;
    v10 = (FMOD::Studio::Bus *)(a1 + 7);
    if ( v8 != nullptr )
    {
      v11 = (FMOD::Studio::Bus *)(a1 + 7);
      while ( 2 )
      {
        v12 = v8;
        while ( *((_QWORD *)v12 + 4) < (unsigned __int64)v16 )
        {
          v12 = *(FMOD::Studio::Bus **)v12;
          if ( v12 == nullptr )
          {
            v12 = v11;
            if ( v11 == v10 )
              goto LABEL_22;
            goto LABEL_19;
          }
        }
        v8 = *((FMOD::Studio::Bus **)v12 + 1);
        v11 = v12;
        if ( v8 != nullptr )
          continue;
        break;
      }
      if ( v12 == v10 )
        goto LABEL_22;
LABEL_19:
      if ( (unsigned __int64)v16 >= *((_QWORD *)v12 + 4) )
        goto LABEL_24;
    }
    else
    {
LABEL_22:
      v12 = (FMOD::Studio::Bus *)(a1 + 7);
    }
    sub_275180(v18, a1 + 6, v12, &v16);
    v12 = v18[0];
LABEL_24:
    *((_QWORD *)v12 + 5) = v9;
  }
  return 0x6365786562696C2FLL;
}
