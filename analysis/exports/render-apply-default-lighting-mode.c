_WORD *__fastcall render_apply_default_lighting_mode(__int64 a1)
{
  _QWORD *v2; // rax
  __int64 v3; // r14
  _WORD *v4; // rax
  _WORD *v5; // r9
  __int64 v6; // rdx
  __int64 v7; // rsi
  _QWORD *v8; // rdi
  bool v9; // zf
  _WORD *result; // rax
  _WORD *v11; // r9
  __int64 v12; // rdx
  __int64 v13; // rsi
  _QWORD *v14; // rdi

  v2 = (_QWORD *)(a1 + 472);
  if ( *(_QWORD *)(a1 + 472) == 0 )
    v2 = (_QWORD *)a1;
  v3 = *v2;
  if ( *v2 != 0 )
    sub_1ADEB0(*v2);
  v4 = *(_WORD **)(a1 + 496);
  v5 = *(_WORD **)(a1 + 504);
  if ( v4 != v5 )
  {
    v6 = *(_QWORD *)(*(_QWORD *)(v3 + 48) + 176LL);
    v7 = unk_1A873B8;
    do
    {
      v8 = (_QWORD *)(*(_QWORD *)(*(_QWORD *)(*(_QWORD *)(v6 + 48LL * ((unsigned __int8)HIBYTE(*v4) >> 4) + 8)
                                            + 8LL * (*(_DWORD *)v4 & 0xFFF))
                                + 56LL)
                    + 16LL);
      do
      {
        v9 = *v8 == v7;
        v8 += 3;
      }
      while ( !v9 );
      *(_BYTE *)(*(v8 - 5) + 22LL) = *(_DWORD *)(a1 + 560) == 0;
      v4 += 2;
    }
    while ( v4 != v5 );
  }
  result = *(_WORD **)(a1 + 528);
  v11 = *(_WORD **)(a1 + 536);
  if ( result != v11 )
  {
    v12 = *(_QWORD *)(*(_QWORD *)(v3 + 48) + 176LL);
    v13 = unk_1A873B8;
    do
    {
      v14 = (_QWORD *)(*(_QWORD *)(*(_QWORD *)(*(_QWORD *)(v12 + 48LL * ((unsigned __int8)HIBYTE(*result) >> 4) + 8)
                                             + 8LL * (*(_DWORD *)result & 0xFFF))
                                 + 56LL)
                     + 16LL);
      do
      {
        v9 = *v14 == v13;
        v14 += 3;
      }
      while ( !v9 );
      *(_BYTE *)(*(v14 - 5) + 22LL) = *(_DWORD *)(a1 + 560) == 1;
      result += 2;
    }
    while ( result != v11 );
  }
  if ( v3 != 0 )
    return (_WORD *)sub_1ADEF0(v3);
  return result;
}
