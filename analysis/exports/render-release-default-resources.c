__int64 __fastcall render_release_default_resources(_QWORD *a1)
{
  __int64 v3; // rdi
  __int64 v4; // rdi
  __int64 v6; // rbx
  __int64 result; // rax
  __int64 v8; // rdi
  __int64 v9; // rdi
  __int64 v10; // rdi
  __int64 v11; // rdi
  __int64 v12; // rdi
  __int64 v13; // rdi
  __int64 v14; // rdi
  __int64 v15; // rdi
  __int64 v16; // rdi

  _R14 = a1;
  v3 = *a1;
  if ( v3 != 0 )
    sub_1ADEF0(v3);
  *_R14 = 0;
  v4 = _R14[59];
  if ( v4 != 0 )
    sub_1ADEF0(v4);
  __asm { vxorps  ymm0, ymm0, ymm0 }
  v6 = 0x1FFFFFFFFFFFFFF9LL;
  __asm
  {
    vmovups ymmword ptr [r14+1C8h], ymm0
    vmovups ymmword ptr [r14+1A8h], ymm0
  }
  _R14[61] = 0;
  _R14[63] = _R14[62];
  result = _R14[66];
  _R14[67] = result;
  do
  {
    v8 = _R14[v6 + 8];
    if ( v8 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v8 + 8LL))(v8);
    _R14[v6 + 8] = 0;
    v9 = _R14[v6 + 15];
    if ( v9 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v9 + 8LL))(v9);
    _R14[v6 + 15] = 0;
    v10 = _R14[v6 + 22];
    if ( v10 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v10 + 8LL))(v10);
    _R14[v6 + 22] = 0;
    v11 = _R14[v6 + 29];
    if ( v11 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v11 + 8LL))(v11);
    _R14[v6 + 29] = 0;
    v12 = _R14[v6 + 36];
    if ( v12 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v12 + 8LL))(v12);
    _R14[v6 + 36] = 0;
    v13 = _R14[v6 + 43];
    if ( v13 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v13 + 8LL))(v13);
    _R14[v6 + 43] = 0;
    v14 = _R14[v6 + 50];
    if ( v14 != 0 )
      result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v14 + 8LL))(v14);
    _R14[v6 + 50] = 0;
    ++v6;
  }
  while ( v6 != 0 );
  v15 = _R14[50];
  if ( v15 != 0 )
    result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v15 + 8LL))(v15);
  _R14[50] = 0;
  v16 = _R14[51];
  if ( v16 != 0 )
    result = (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)v16 + 8LL))(v16);
  _R14[51] = 0;
  return result;
}
