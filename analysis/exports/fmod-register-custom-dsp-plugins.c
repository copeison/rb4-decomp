__int64 __fastcall fmod_register_custom_dsp_plugins(__int64 a1)
{
  __int64 v2; // r14
  void *analysis_dsp_description; // rax
  __int64 v4; // r14
  void *bitcrusher_dsp_description; // rax
  __int64 v6; // r14
  double v7; // xmm0_8
  __m128 v8; // xmm1
  double v9; // xmm2_8
  __m128 v10; // xmm3
  void *delay_dsp_description; // rax
  __int64 v12; // r14
  double v13; // xmm0_8
  __m128 v14; // xmm1
  double v15; // xmm2_8
  __m128 v16; // xmm3
  void *filter_dsp_description; // rax
  __int64 v18; // r14
  void *gain_dsp_description; // rax
  __int64 v20; // r14
  void *signal_tap_dsp_description; // rax
  __int64 v22; // r14
  void *pitch_shift_dsp_description; // rax
  __int64 v24; // r14
  void *stutter_dsp_description; // rax
  __int64 v26; // r14
  void *sound_clash_slot_dsp_description; // rax
  __int64 v28; // r14
  void *tremolo_dsp_description; // rax
  __int64 v30; // r14
  void *vibe_dsp_description; // rax
  __int64 v32; // rbx
  void *wah_dsp_description; // rax

  v2 = *(_QWORD *)(a1 + 280);
  analysis_dsp_description = fmod_get_analysis_dsp_description();
  FMOD::Studio::System::registerPlugin(v2, analysis_dsp_description);
  v4 = *(_QWORD *)(a1 + 280);
  bitcrusher_dsp_description = fmod_get_bitcrusher_dsp_description();
  FMOD::Studio::System::registerPlugin(v4, bitcrusher_dsp_description);
  v6 = *(_QWORD *)(a1 + 280);
  delay_dsp_description = fmod_get_delay_dsp_description(v7, v8, v9, v10);
  FMOD::Studio::System::registerPlugin(v6, delay_dsp_description);
  v12 = *(_QWORD *)(a1 + 280);
  filter_dsp_description = fmod_get_filter_dsp_description(v13, v14, v15, v16);
  FMOD::Studio::System::registerPlugin(v12, filter_dsp_description);
  v18 = *(_QWORD *)(a1 + 280);
  gain_dsp_description = fmod_get_gain_dsp_description();
  FMOD::Studio::System::registerPlugin(v18, gain_dsp_description);
  v20 = *(_QWORD *)(a1 + 280);
  signal_tap_dsp_description = fmod_get_signal_tap_dsp_description();
  FMOD::Studio::System::registerPlugin(v20, signal_tap_dsp_description);
  v22 = *(_QWORD *)(a1 + 280);
  pitch_shift_dsp_description = fmod_get_pitch_shift_dsp_description();
  FMOD::Studio::System::registerPlugin(v22, pitch_shift_dsp_description);
  v24 = *(_QWORD *)(a1 + 280);
  stutter_dsp_description = fmod_get_stutter_dsp_description();
  FMOD::Studio::System::registerPlugin(v24, stutter_dsp_description);
  v26 = *(_QWORD *)(a1 + 280);
  sound_clash_slot_dsp_description = fmod_get_sound_clash_slot_dsp_description();
  FMOD::Studio::System::registerPlugin(v26, sound_clash_slot_dsp_description);
  v28 = *(_QWORD *)(a1 + 280);
  tremolo_dsp_description = fmod_get_tremolo_dsp_description();
  FMOD::Studio::System::registerPlugin(v28, tremolo_dsp_description);
  v30 = *(_QWORD *)(a1 + 280);
  vibe_dsp_description = fmod_get_vibe_dsp_description();
  FMOD::Studio::System::registerPlugin(v30, vibe_dsp_description);
  v32 = *(_QWORD *)(a1 + 280);
  wah_dsp_description = fmod_get_wah_dsp_description();
  FMOD::Studio::System::registerPlugin(v32, wah_dsp_description);
  return 0;
}
