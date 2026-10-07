// Checks the named record driver for connection and disconnects the slot when it disappears.
__int64 __fastcall fmod_audio_input_device_refresh_record_driver(__int64 a1)
{
  __int64 v2; // r13
  int v3; // ebx
  _BYTE *v4; // r14
  __int64 v5; // r13
  unsigned int v6; // ebx
  int RecordDriverInfo; // eax
  int v8; // ecx
  int v9; // eax
  __int64 v11; // [rsp+0h] [rbp-170h]
  int v12; // [rsp+10h] [rbp-160h] BYREF
  _BYTE v13[4]; // [rsp+14h] [rbp-15Ch] BYREF
  int v14; // [rsp+18h] [rbp-158h] BYREF
  int v15; // [rsp+1Ch] [rbp-154h] BYREF
  int v16; // [rsp+20h] [rbp-150h] BYREF
  int v17; // [rsp+24h] [rbp-14Ch] BYREF
  __int64 v18; // [rsp+28h] [rbp-148h] BYREF
  _BYTE v19[16]; // [rsp+30h] [rbp-140h] BYREF
  _BYTE v20[256]; // [rsp+40h] [rbp-130h] BYREF
  __int64 v21; // [rsp+140h] [rbp-30h]

  v2 = a1 + 16632;
  v21 = 0x6365786562696C2FLL;
  scePthreadMutexLock(a1 + 16632);
  v3 = *(_DWORD *)(a1 + 16624) + 1;
  *(_DWORD *)(a1 + 16624) = v3;
  if ( (unsigned int)strcmp(*(_QWORD *)(a1 + 16608), 19246190) == 0 )
  {
    LODWORD(v4) = 0;
  }
  else
  {
    v11 = v2;
    v5 = *(_QWORD *)(unk_19F29D8 + 288LL);
    v17 = 0;
    v16 = 0;
    FMOD::System::getRecordNumDrivers(v5, &v17, &v16);
    v15 = 0;
    v14 = 0;
    if ( v17 <= 0 )
      goto LABEL_12;
    v6 = 0;
    v4 = v20;
    while ( 1 )
    {
      RecordDriverInfo = FMOD::System::getRecordDriverInfo(v5, v6, v20, 256, v19, &v15, v13, &v14, &v12);
      v8 = 4;
      if ( RecordDriverInfo == 0 )
      {
        v9 = strcmp(*(_QWORD *)(a1 + 16608), v20);
        v8 = 0;
        if ( v9 == 0 )
          v8 = 2 - (v12 & 1);
      }
      if ( (v8 & 3 | 4) != 4 )
        break;
      if ( (int)++v6 >= v17 )
        goto LABEL_12;
    }
    LOBYTE(v4) = 1;
    if ( v8 == 2 )
    {
LABEL_12:
      sub_E16B0(a1);
      sub_256FD0(&v18, 19246190);
      LODWORD(v4) = 0;
      *(_QWORD *)(a1 + 16608) = v18;
      *(_DWORD *)(a1 + 16600) = -1;
    }
    v3 = *(_DWORD *)(a1 + 16624);
    v2 = v11;
  }
  *(_DWORD *)(a1 + 16624) = v3 - 1;
  scePthreadMutexUnlock(v2);
  return (unsigned int)v4;
}
