bool __fastcall special_pad_reader_set_calibration_mode(void *self, unsigned int mode)
{
  _DWORD *v4; // r15
  __int16 v5; // cx
  __int16 v6; // dx
  bool v7; // r13
  int v8; // r12d
  int data; // [rsp+8h] [rbp-48h] BYREF
  char v11; // [rsp+Ch] [rbp-44h]
  _WORD open_param[8]; // [rsp+10h] [rbp-40h] BYREF
  __int64 v13; // [rsp+20h] [rbp-30h]

  v13 = 0x6365786562696C2FLL;
  v4 = (_DWORD *)sub_8CF3A0(self);
  scePthreadMutexLock(v4 + 2);
  ++*v4;
  *((_DWORD *)self + 2049) = mode;
  *((_DWORD *)self + 2050) = 0;
  *((_DWORD *)self + 2116) = 0;
  v5 = 1848;
  v6 = -32159;
  if ( *((_DWORD *)self + 2117) == 32 )
  {
    v5 = 3695;
    v6 = 371;
  }
  open_param[0] = v5;
  v7 = false;
  open_param[1] = v6;
  open_param[2] = v6;
  v8 = scePadOpenExt(*((_DWORD *)self + 2), 2, 0, open_param);
  if ( v8 >= 0 )
  {
    data = 524592;
    if ( mode <= 2 )
      v11 = 0x43FF01u >> (8 * mode);
    if ( scePadSetFeatureReport(v8, 0x30u, &data, 5u) != 0
      && (usleep(1000), scePadSetFeatureReport(v8, 0x30u, &data, 5u) != 0) )
    {
      scePadClose((unsigned int)v8);
      v7 = false;
    }
    else
    {
      scePadClose((unsigned int)v8);
      v7 = true;
    }
  }
  --*v4;
  scePthreadMutexUnlock(v4 + 2);
  return v7;
}
