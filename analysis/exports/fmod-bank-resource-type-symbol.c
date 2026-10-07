__int64 fmod_bank_resource_type_symbol()
{
  __int64 result; // rax
  _QWORD v1[4]; // [rsp+0h] [rbp-20h] BYREF

  v1[1] = 0x6365786562696C2FLL;
  if ( byte_19F2A08[0] == 0 && (unsigned int)_cxa_guard_acquire(byte_19F2A08) != 0 )
  {
    unk_19F2A00 = 19246190;
    _cxa_guard_release(byte_19F2A08);
  }
  result = unk_19F2A00;
  if ( unk_19F2A00 == 19246190 )
  {
    sub_256FD0(v1, "FModBankResource");
    unk_19F2A00 = v1[0];
    return v1[0];
  }
  return result;
}
