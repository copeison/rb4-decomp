// Runs one main-loop iteration across input, platform, UI, audio, and game systems. Returns whether another frame should run. Name is inferred from its sole looping caller.
bool game_run_frame()
{
  __int64 v0; // rax
  __int64 v1; // rdi
  __int64 v2; // rsi
  _QWORD v4[4]; // [rsp+0h] [rbp-60h] BYREF
  _QWORD *v5; // [rsp+20h] [rbp-40h]
  __int64 v6; // [rsp+38h] [rbp-28h]

  v6 = 0x6365786562696C2FLL;
  sub_369940();
  sub_7560(&unk_19C5638);
  if ( unk_1ADF350 != 0 )
    sub_8ECAC0();
  sub_33B510(&qword_19FBAF0);
  sub_D20C10(&unk_1B15288);
  sub_D2C440(&dword_1B156A0);
  sub_F2CE50(unk_1B43A10);
  sub_AE3880(unk_1AF4E38);
  sub_D1E2C0(&unk_1B15110);
  sub_8F3220(unk_1ADF7D0);
  sub_D5C230(unk_1B1F7F0);
  sub_ADE120();
  sub_92F4A0(unk_1ADFE28);
  sub_B0A070();
  sub_8C8900(unk_1AC9A78);
  if ( unk_1AEF778 != 0 )
    sub_A47EC0();
  sub_BB58C0(&unk_1AFF588);
  sub_3AE960(unk_1A6E0A8);
  sub_C60720(unk_1B094D8);
  sub_B0CE80(unk_1AFA328);
  sub_BD9430(unk_1B01798);
  sub_928FC0(&unk_1ADFB78);
  sub_3DE0E0(unk_1A712A0);
  if ( unk_1A712A0 == 0 )
    return false;
  v0 = sub_8CA4C0(unk_1AC9A78);
  if ( v0 != 0 && (unsigned __int8)sub_8B1840(v0) != 0 )
  {
    sub_3DEAA0(unk_1A712A0);
  }
  else
  {
    if ( (unsigned __int8)sub_43B130() != 0 )
    {
      v1 = *(_QWORD *)(unk_1A712A0 + 112LL);
      v2 = *(unsigned int *)(*(_QWORD *)(unk_1A712A0 + 296LL) + 184LL);
      v4[0] = &unk_18DC010;
      v5 = v4;
      sub_43B140(v1, v2, v4);
      if ( v5 != nullptr )
      {
        (*(void (__fastcall **)(_QWORD *, bool))(*v5 + 32LL))(v5, v5 != v4);
        v5 = nullptr;
      }
    }
    if ( (unsigned __int8)sub_3DE130(unk_1A712A0) != 0 )
    {
      sub_8C9A80(unk_1AC9A78);
      sub_3DE7C0(unk_1A712A0);
    }
  }
  return unk_19C54C8 == 0 && unk_1A712A0 != 0;
}
