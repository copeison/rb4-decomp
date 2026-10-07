__int64 __fastcall fmod_bank_resource_load(_QWORD *a1, char a2)
{
  char v5; // bl
  char v6; // bl
  __int64 v7; // rsi
  double v8; // xmm0_8
  char v9; // bl
  char v10; // bl
  double v12; // xmm0_8
  FMOD::Studio::System **v13; // r13
  FMOD::Studio::System **i; // r14
  char v15; // bl
  __int64 v16; // rsi
  double v17; // xmm0_8
  __int64 v18; // rbx
  char v19; // bl
  __int64 v20; // rsi
  double v21; // xmm0_8
  __int64 v22; // rbx
  _BYTE v24[16]; // [rsp+0h] [rbp-F0h] BYREF
  _BYTE v25[8]; // [rsp+10h] [rbp-E0h] BYREF
  __int64 v26; // [rsp+18h] [rbp-D8h]
  __int64 v27; // [rsp+20h] [rbp-D0h] BYREF
  __int64 v28; // [rsp+28h] [rbp-C8h] BYREF
  _BYTE v29[16]; // [rsp+30h] [rbp-C0h] BYREF
  _QWORD v30[2]; // [rsp+40h] [rbp-B0h] BYREF
  __int64 v31; // [rsp+50h] [rbp-A0h] BYREF
  __int64 v32; // [rsp+58h] [rbp-98h] BYREF
  _BYTE v33[8]; // [rsp+60h] [rbp-90h] BYREF
  __int64 v34; // [rsp+68h] [rbp-88h]
  __int64 v35; // [rsp+70h] [rbp-80h] BYREF
  __int64 v36; // [rsp+78h] [rbp-78h] BYREF
  _QWORD v37[2]; // [rsp+80h] [rbp-70h] BYREF
  _BYTE v38[16]; // [rsp+90h] [rbp-60h] BYREF
  __int128 v39; // [rsp+A0h] [rbp-50h] BYREF
  __int64 v40; // [rsp+B0h] [rbp-40h]
  _BYTE v41[8]; // [rsp+B8h] [rbp-38h] BYREF
  __int64 v42; // [rsp+C0h] [rbp-30h]

  v42 = 0x6365786562696C2FLL;
  if ( unk_19E4559 == 0 )
  {
    sub_255080(v37);
    sub_2553B0(v37, 512);
    fmod_bank_resource_resolve_platform_path((__int64)a1, v37);
    if ( a2 == 0 )
    {
      sub_2551E0(&v39, v37);
      sub_254950(&v39);
      v5 = sub_2548C0(&v39, "master bank.bank");
      sub_255550(&v39);
      if ( v5 == 0 )
      {
        sub_2551E0(&v39, v37);
        sub_254950(&v39);
        v6 = sub_2548C0(&v39, "master bank.strings.bank");
        sub_255550(&v39);
        if ( v6 == 0
          && (qword_19F2E70 == 0
           || (*(unsigned __int8 (__fastcall **)(__int64))(*(_QWORD *)qword_19F2E70 + 48LL))(qword_19F2E70) != 0) )
        {
          fmod_bank_resource_make_language_bank_path(v33, v7, v37);
          v35 = 19246190;
          v8 = sub_1AF950(&v35, v34);
          sub_264F70(&v36, v35, 0, v8);
          if ( v36 != 0 )
            sub_1ADEF0(v36);
          sub_255550(v33);
        }
      }
      sub_2551E0(&v39, v37);
      sub_254950(&v39);
      v9 = sub_2548C0(&v39, "master bank.bank");
      sub_255550(&v39);
      if ( v9 != 0 && byte_19F2ED9 != 1
        || (sub_2551E0(&v39, v37),
            sub_254950(&v39),
            v10 = sub_2548C0(&v39, "master bank.strings.bank"),
            sub_255550(&v39),
            v10 != 0)
        && byte_19F2ED8 == 0 )
      {
        if ( qword_19F2E70 != 0 )
          sub_1ADEF0(qword_19F2E70);
        qword_19F2E70 = 0;
        if ( qword_19F2E78 != 0 )
          sub_1ADEF0(qword_19F2E78);
        qword_19F2E78 = 0;
      }
    }
    __asm { vxorps  xmm0, xmm0, xmm0 }
    __asm { vmovups [rbp+var_50], xmm0 }
    v40 = 0;
    v12 = nullsub_18(v41, "EASTL vector");
    sub_276E00(&v39, v12);
    v13 = *((FMOD::Studio::System ***)&v39 + 1);
    for ( i = (FMOD::Studio::System **)v39; v13 != i; ++i )
      fmod_bank_resource_load_into_studio_system(a1, (__int64)v37, *i);
    if ( a2 == 0 )
    {
      sub_2551E0(v38, v37);
      sub_254950(v38);
      v15 = sub_2548C0(v38, "master bank.bank");
      sub_255550(v38);
      if ( v15 != 0 )
      {
        if ( a1 != nullptr )
          sub_1ADEB0(a1);
        if ( qword_19F2E70 != 0 )
          sub_1ADEF0(qword_19F2E70);
        qword_19F2E70 = (__int64)a1;
        if ( byte_19F2ED9 == 0 )
        {
          byte_19F2ED8 = 1;
          sub_255150(v29, a1[1]);
          fmod_bank_resource_make_strings_bank_path(v30, v16, (__int64)v29);
          v31 = 19246190;
          v17 = sub_1AF950(&v31, v30[1]);
          sub_264F70(&v32, v31, 0, v17);
          v18 = v32;
          v32 = 0;
          if ( qword_19F2E78 != 0 )
          {
            sub_1ADEF0(qword_19F2E78);
            qword_19F2E78 = v18;
            if ( v32 != 0 )
              sub_1ADEF0(v32);
          }
          else
          {
            qword_19F2E78 = v18;
          }
          sub_255550(v30);
          sub_255550(v29);
          byte_19F2ED8 = 0;
        }
      }
      else
      {
        sub_2551E0(v38, v37);
        sub_254950(v38);
        v19 = sub_2548C0(v38, "master bank.strings.bank");
        sub_255550(v38);
        if ( v19 != 0 )
        {
          if ( a1 != nullptr )
            sub_1ADEB0(a1);
          if ( qword_19F2E78 != 0 )
            sub_1ADEF0(qword_19F2E78);
          qword_19F2E78 = (__int64)a1;
          if ( byte_19F2ED8 == 0 )
          {
            byte_19F2ED9 = 1;
            sub_255150(v24, a1[1]);
            fmod_bank_resource_make_master_bank_path(v25, v20, v24);
            v27 = 19246190;
            v21 = sub_1AF950(&v27, v26);
            sub_264F70(&v28, v27, 0, v21);
            v22 = v28;
            v28 = 0;
            if ( qword_19F2E70 != 0 )
            {
              sub_1ADEF0(qword_19F2E70);
              qword_19F2E70 = v22;
              if ( v28 != 0 )
                sub_1ADEF0(v28);
            }
            else
            {
              qword_19F2E70 = v22;
            }
            sub_255550(v25);
            sub_255550(v24);
            byte_19F2ED9 = 0;
          }
        }
      }
    }
    if ( (_QWORD)v39 != 0 )
      sub_252D30(v41, v39, v40 - v39);
    sub_255550(v37);
  }
  return 0x6365786562696C2FLL;
}
