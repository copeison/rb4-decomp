// Initializes the renderer backend, built-in resources, frame owners, and runtime epoch.
unsigned __int64 __fastcall render_system_initialize(__int64 a1, __int64 a2)
{
  __int64 v4; // r15
  __m128 v6; // xmm0
  __int64 v7; // r14
  __int64 v8; // r14
  __int64 v9; // rax
  unsigned __int64 v10; // rbx
  unsigned __int64 result; // rax
  unsigned int v12; // edx

  _R12 = (__int64 *)a1;
  _R14 = a2;
  *(_BYTE *)(a1 + 32) = 1;
  v4 = a1 + 2544;
  __asm { vmovups xmm0, xmmword ptr [r14] }
  __asm { vmovups xmmword ptr [r12+28h], xmm0 }
  sub_63F400(a1 + 2544);
  (*(void (__fastcall **)(__int64 *, __int64))(*_R12 + 24))(_R12, _R14);
  sub_641370(v4);
  sub_47F030(_R12 + 407, v6);
  sub_451C90(_R12 + 445);
  v7 = sub_37BF40(40);
  sub_460640(v7);
  _R12[446] = v7;
  v8 = sub_37BF40(72);
  sub_457620(v8);
  _R12[447] = v8;
  sub_62ACB0(_R12 + 448);
  render_system_initialize_builtin_buffers(_R12);
  (*(void (__fastcall **)(__int64))(*(_QWORD *)_R12[7] + 16LL))(_R12[7]);
  v9 = _R12[9];
  if ( _R12[10] != v9 )
  {
    v10 = 0;
    do
    {
      (*(void (__fastcall **)(_QWORD))(**(_QWORD **)(v9 + 8 * v10) + 16LL))(*(_QWORD *)(v9 + 8 * v10));
      v9 = _R12[9];
      ++v10;
    }
    while ( v10 < (_R12[10] - v9) >> 3 );
  }
  (*(void (__fastcall **)(__int64 *))(*_R12 + 32))(_R12);
  result = *((unsigned int *)_R12 + 68);
  if ( (result & 0x80000000) == 0LL )
  {
    *((_DWORD *)_R12 + 68) = result + 1;
    if ( (_DWORD)result == 0 )
    {
      result = __rdtsc();
      v12 = HIDWORD(result);
      result = (unsigned int)result;
      _R12[32] = (unsigned int)result | ((unsigned __int64)v12 << 32);
    }
  }
  return result;
}
