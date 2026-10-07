void __fastcall special_pad_reader_open(void *self, int user_id)
{
  __int64 v2; // rax
  _DWORD *v4; // r14
  int v5; // eax
  unsigned int v6; // ebx
  bool v7; // r12
  int v8; // eax
  unsigned int v9; // ebx
  _BYTE info[11]; // [rsp+0h] [rbp-68h] BYREF
  char v11; // [rsp+Bh] [rbp-5Dh]
  int open_param; // [rsp+30h] [rbp-38h] BYREF
  __int16 v13; // [rsp+34h] [rbp-34h]
  __int64 v14; // [rsp+40h] [rbp-28h]
  __int64 v15; // [rsp+48h] [rbp-20h]
  __int64 v16; // [rsp+58h] [rbp-10h]
  __int64 v17; // [rsp+60h] [rbp-8h]
  __int64 savedregs; // [rsp+68h] [rbp+0h] BYREF

  v16 = v2;
  sub_8D02F0(self, user_id, 1u);
  savedregs = (__int64)&savedregs;
  v15 = v17;
  v4 = self;
  v14 = 0x6365786562696C2FLL;
  open_param = -2107570376;
  v13 = -32159;
  v5 = scePadOpenExt(*((_DWORD *)self + 2), 2, 0, &open_param);
  v6 = v5;
  v7 = v5 >= 0;
  if ( v5 >= 0 )
  {
    scePadGetExtControllerInformation(v5, info);
    v7 = v11 != 0;
    if ( v11 != 0 )
      v4[2117] = 31;
  }
  scePadClose(v6);
  if ( !v7 )
  {
    open_param = (int)&byte_1730E6F;
    v13 = 371;
    v8 = scePadOpenExt(v4[2], 2, 0, &open_param);
    v9 = v8;
    if ( v8 >= 0 )
    {
      scePadGetExtControllerInformation(v8, info);
      if ( v11 != 0 )
        v4[2117] = 32;
    }
    scePadClose(v9);
  }
}
