// Reconciles connected FMOD record drivers ending in GENERAL with the fixed input-device pool.
__int64 __fastcall fmod_audio_input_manager_refresh_record_devices(__int64 a1)
{
  __int64 *v2; // rbx
  __int64 *v3; // r15
  __int64 v4; // r14
  __int64 v5; // r12
  unsigned int i; // ebx
  _QWORD *v7; // r15
  _QWORD *v8; // r13
  __int64 v9; // r14
  __int64 v10; // r14
  unsigned __int64 v11; // rax
  __int64 v13; // [rsp+8h] [rbp-168h]
  __int64 v14; // [rsp+10h] [rbp-160h] BYREF
  char v15[4]; // [rsp+18h] [rbp-158h] BYREF
  char v16[4]; // [rsp+1Ch] [rbp-154h] BYREF
  int v17; // [rsp+20h] [rbp-150h] BYREF
  int v18; // [rsp+24h] [rbp-14Ch] BYREF
  int v19; // [rsp+28h] [rbp-148h] BYREF
  int v20; // [rsp+2Ch] [rbp-144h] BYREF
  _BYTE v21[16]; // [rsp+30h] [rbp-140h] BYREF
  _BYTE v22[256]; // [rsp+40h] [rbp-130h] BYREF
  __int64 v23; // [rsp+140h] [rbp-30h]

  v23 = 0x6365786562696C2FLL;
  v2 = (__int64 *)unk_19C8FE0;
  v3 = (__int64 *)unk_19C8FE8;
  if ( unk_19C8FE0 != unk_19C8FE8 )
  {
    do
    {
      v4 = *v2;
      if ( (*(unsigned int (__fastcall **)(__int64))(*(_QWORD *)*v2 + 24LL))(*v2) == 1
        && *(_QWORD *)(v4 + 16608) != 19246190
        && (unsigned __int8)fmod_audio_input_device_refresh_record_driver(v4) == 0 )
      {
        sub_A2DE0(a1);
      }
      ++v2;
    }
    while ( v3 != v2 );
  }
  v13 = a1;
  v5 = *(_QWORD *)(unk_19F29D8 + 288LL);
  v20 = 0;
  v19 = 0;
  v18 = 0;
  v17 = 0;
  FMOD::System::getRecordNumDrivers(v5, &v20, &v19);
  if ( v20 > 0 )
  {
    for ( i = 0; (int)i < v20; ++i )
    {
      if ( (unsigned int)FMOD::System::getRecordDriverInfo(v5, i, v22, 256, v21, &v18, v16, &v17, v15) == 0
        && (v15[0] & 1) != 0 )
      {
        v11 = strlen(v22);
        if ( v11 >= 7 && (unsigned int)strncmp(&v21[v11 + 9], "GENERAL", 7) == 0 )
        {
          v7 = (_QWORD *)unk_19C8FE0;
          v8 = (_QWORD *)unk_19C8FE8;
          if ( unk_19C8FE0 == unk_19C8FE8 )
          {
LABEL_13:
            v10 = sub_A26F0(&unk_19C8FC0, 1);
            if ( v10 != 0 )
            {
              sub_256FD0(&v14, v22);
              if ( fmod_audio_input_device_bind_record_driver(v10, i, v14) != 0 )
                sub_A2DE0(v13);
            }
          }
          else
          {
            while ( 1 )
            {
              v9 = *v7;
              if ( (*(unsigned int (__fastcall **)(_QWORD))(*(_QWORD *)*v7 + 24LL))(*v7) == 1
                && (unsigned int)strcmp(*(_QWORD *)(v9 + 16608), v22) == 0 )
              {
                break;
              }
              if ( v8 == ++v7 )
                goto LABEL_13;
            }
          }
        }
      }
    }
  }
  return 0x6365786562696C2FLL;
}
