// Returns low-level channel position or Studio event timeline position in milliseconds.
float __fastcall audio_clip_fmod_get_position_ms(void *clip)
{
  __int64 v1; // rdx
  __int64 v2; // rsi
  FMOD::Studio::EventInstance *v5; // rdi
  float result; // xmm0_4
  int v8; // [rsp+Ch] [rbp-14h] BYREF
  __int64 v9; // [rsp+10h] [rbp-10h]

  v9 = 0x6365786562696C2FLL;
  if ( *((_QWORD *)clip + 50) != 0 )
  {
    *(float *)&_XMM0 = (*(float (__fastcall **)(void *, __int64, __int64, __int64))(*(_QWORD *)clip + 32LL))(
                         clip,
                         v2,
                         v1,
                         0x6365786562696C2FLL);
  }
  else
  {
    v5 = *((FMOD::Studio::EventInstance **)clip + 53);
    if ( v5 != nullptr )
    {
      *(double *)&_XMM0 = FMOD::Studio::EventInstance::getTimelinePosition(v5, &v8);
      __asm { vcvtsi2ss xmm0, xmm0, [rbp+var_14] }
    }
    else
    {
      __asm { vxorps  xmm0, xmm0, xmm0 }
    }
  }
  LODWORD(result) = _XMM0;
  return result;
}
