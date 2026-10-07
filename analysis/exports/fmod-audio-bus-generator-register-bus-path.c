__int64 __fastcall fmod_audio_bus_generator_register_bus_path(__int64 a1, FMOD::Studio::Bus *a2)
{
  _QWORD *v4; // rbx
  void (__fastcall *v5)(_QWORD *, __int64, double); // r12
  double v6; // xmm0_8
  __int64 v8; // [rsp+18h] [rbp-148h] BYREF
  __m256 v9; // [rsp+20h] [rbp-140h] BYREF
  __int64 v17; // [rsp+128h] [rbp-38h]

  __asm { vxorps  ymm0, ymm0, ymm0 }
  v17 = 0x6365786562696C2FLL;
  v4 = qword_19C9088;
  v5 = *(void (__fastcall **)(_QWORD *, __int64, double))(*qword_19C9088 + 16LL);
  __asm
  {
    vmovups [rsp+160h+var_60], ymm0
    vmovups [rsp+160h+var_80], ymm0
    vmovups [rsp+160h+var_A0], ymm0
    vmovups [rsp+160h+var_C0], ymm0
    vmovups [rsp+160h+var_E0], ymm0
    vmovups [rsp+160h+var_100], ymm0
    vmovups [rsp+160h+var_120], ymm0
    vmovups [rsp+160h+var_140], ymm0
  }
  FMOD::Studio::Bus::getPath(a2, (char *)&v9, 256, nullptr);
  v6 = sub_256FD0(&v8, &v9);
  v5(v4, v8, v6);
  return 0x6365786562696C2FLL;
}
