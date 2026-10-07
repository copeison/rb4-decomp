// Binds a device slot only when FMOD still reports the expected record-driver name.
char __fastcall fmod_audio_input_device_bind_record_driver(__int64 a1, __int64 a2, __int64 a3)
{
  int v5; // ebx
  __int64 v6; // rdi
  _BYTE v10[4]; // [rsp+8h] [rbp-150h] BYREF
  _BYTE v11[4]; // [rsp+Ch] [rbp-14Ch] BYREF
  int v12; // [rsp+10h] [rbp-148h] BYREF
  int v13; // [rsp+14h] [rbp-144h] BYREF
  _BYTE v14[16]; // [rsp+18h] [rbp-140h] BYREF
  _BYTE v15[264]; // [rsp+28h] [rbp-130h] BYREF
  __int64 v16; // [rsp+130h] [rbp-28h]

  _R14 = a1;
  v5 = a2;
  v16 = 0x6365786562696C2FLL;
  v6 = *(_QWORD *)(unk_19F29D8 + 288LL);
  v13 = 0;
  v12 = 0;
  if ( (unsigned int)FMOD::System::getRecordDriverInfo(v6, a2, v15, 256, v14, &v13, v11, &v12, v10) != 0 )
    return 0;
  if ( (unsigned int)strcmp(a3, v15) != 0 )
    return 0;
  *(_QWORD *)(_R14 + 16608) = a3;
  *(_DWORD *)(_R14 + 16600) = v5;
  __asm { vcvtsi2ss xmm0, xmm0, eax }
  *(_DWORD *)(_R14 + 8) = v13;
  __asm { vmovss  dword ptr [r14+40E8h], xmm0 }
  return 1;
}
