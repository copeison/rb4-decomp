// Replaces input bus routes, resolves every Studio bus, and caches its authored volume.
__int64 __fastcall fmod_audio_input_manager_set_bus_paths(__int64 a1, __int64 *a2)
{
  __int64 v2; // r14
  _QWORD *v4; // rbx
  _QWORD *v5; // r13
  _QWORD *v6; // r15
  __int64 v7; // rcx
  __int64 v8; // rax
  unsigned __int64 v9; // rdi
  unsigned __int64 v10; // rsi
  bool v11; // cc
  unsigned __int64 v12; // rsi
  __int64 v13; // rcx
  __int64 v14; // rdx
  __int64 v15; // rax

  v2 = a1;
  v4 = *(_QWORD **)(a1 + 16);
  v5 = *(_QWORD **)(a1 + 24);
  v6 = (_QWORD *)(a1 + 16);
  if ( v5 != v4 )
  {
    do
    {
      (*(void (__fastcall **)(_QWORD *, _QWORD))(*qword_19C9088 + 24LL))(qword_19C9088, *v4);
      v4 += 3;
    }
    while ( v5 != v4 );
    v4 = (_QWORD *)*v6;
    v2 = a1;
    v5 = (_QWORD *)*v6;
    *(_QWORD *)(a1 + 24) = *v6;
  }
  v7 = a2[1];
  v8 = *a2;
  v9 = 0xAAAAAAAAAAAAAAABLL * (v5 - v4);
  v10 = (v7 - *a2) >> 3;
  v11 = v10 <= v9;
  v12 = v10 - v9;
  if ( v11 )
  {
    *(_QWORD *)(v2 + 24) = &v4[3 * ((a2[1] - *a2) >> 3)];
  }
  else
  {
    sub_275C10(v6, v12);
    v8 = *a2;
    v7 = a2[1];
  }
  if ( (int)((unsigned __int64)(v7 - v8) >> 3) > 0 )
  {
    v13 = 0;
    v14 = 0;
    do
    {
      v15 = *(_QWORD *)(v8 + 8 * v14++);
      *(_QWORD *)(*v6 + v13) = v15;
      v13 += 24;
      v8 = *a2;
    }
    while ( v14 < (int)((unsigned __int64)(a2[1] - *a2) >> 3) );
  }
  return fmod_audio_input_manager_bind_buses(v2);
}
