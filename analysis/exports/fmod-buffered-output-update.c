int __fastcall fmod_buffered_output_update(void *output_state)
{
  __int64 v1; // rax
  __int64 v2; // rdi
  _QWORD v4[3]; // [rsp+8h] [rbp-18h] BYREF

  v4[1] = 0x6365786562696C2FLL;
  v1 = *(_QWORD *)output_state;
  v4[0] = output_state;
  v2 = *(_QWORD *)(v1 + 352);
  if ( v2 == 0 )
  {
    std::_Xbad_function_call();
    BUG();
  }
  return (*(int (__fastcall **)(__int64, _QWORD *))(*(_QWORD *)v2 + 16LL))(v2, v4);
}
