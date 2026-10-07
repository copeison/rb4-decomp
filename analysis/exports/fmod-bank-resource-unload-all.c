__int64 __fastcall fmod_bank_resource_unload_all(_QWORD *a1, double a2)
{
  _QWORD *v2; // rbx
  __int64 v3; // rax
  __int64 v4; // r15
  __int64 v5; // r12
  FMOD::Studio::Bank *v6; // r14
  int LoadingState; // eax
  int v8; // ebx
  __int64 v9; // rdi
  _QWORD *v12; // [rsp+8h] [rbp-48h]
  _QWORD *v13; // [rsp+10h] [rbp-40h]
  int v14; // [rsp+1Ch] [rbp-34h] BYREF
  __int64 v15; // [rsp+20h] [rbp-30h]

  v2 = a1;
  v15 = 0x6365786562696C2FLL;
  v3 = a1[11];
  if ( v3 != 0 )
  {
    v12 = a1 + 7;
    v13 = a1 + 12;
    do
    {
      v4 = v2[8];
      v5 = *(_QWORD *)(v4 + 32);
      v6 = *(FMOD::Studio::Bank **)(v4 + 40);
      v2[11] = v3 - 1;
      sub_253100(v4);
      sub_2534A0(v4, v12);
      sub_252D30(v13, v4, 48);
      FMOD::Studio::Bank::unload(v6);
      LoadingState = FMOD::Studio::Bank::getLoadingState(v6, &v14);
      if ( (v14 | LoadingState) == 0 )
      {
        do
        {
          scePthreadYield();
          v8 = FMOD::Studio::Bank::getLoadingState(v6, &v14);
          FMOD::Studio::System::update(v5);
        }
        while ( (v14 | v8) == 0 );
      }
      v2 = a1;
      v3 = a1[11];
    }
    while ( v3 != 0 );
  }
  v9 = v2[13];
  if ( v9 != 0 )
  {
    sub_37B800(v9, a2);
    v2[13] = 0;
  }
  return 0x6365786562696C2FLL;
}
