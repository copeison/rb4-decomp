__int64 __fastcall fmod_audio_input_manager_clear_bus_paths(__int64 a1)
{
  _QWORD *v2; // rbx
  _QWORD *v3; // r15
  __int64 result; // rax

  v2 = *(_QWORD **)(a1 + 16);
  v3 = *(_QWORD **)(a1 + 24);
  if ( v3 != v2 )
  {
    do
    {
      result = (*(__int64 (__fastcall **)(_QWORD *, _QWORD))(*qword_19C9088 + 24LL))(qword_19C9088, *v2);
      v2 += 3;
    }
    while ( v3 != v2 );
    *(_QWORD *)(a1 + 24) = *(_QWORD *)(a1 + 16);
  }
  return result;
}
