/* Generated Hex-Rays evidence for the common render constant-buffer lifecycle. */

/* 0x639F30 */
// Creates a platform constant buffer, using the descriptor count for size_t(-1), and performs eager upload unless flag bit 0 defers it.
_BYTE *__fastcall render_create_constant_buffer(__int64 a1, unsigned int a2, __int64 a3)
{
  char v3; // r14
  __int64 v4; // rax
  _BYTE *v5; // rax
  _BYTE *v6; // rbx

  v3 = a2;
  v4 = a3;
  if ( a3 == -1 )
    v4 = *(_QWORD *)(a1 + 24);
  v5 = (_BYTE *)(*(__int64 (__fastcall **)(_QWORD, __int64, _QWORD, __int64))(**(_QWORD **)(g_render_system + 304) + 88LL))(
                  *(_QWORD *)(g_render_system + 304),
                  a1,
                  a2,
                  v4);
  v6 = v5;
  if ( (v3 & 1) == 0 && v5[56] != 0 )
  {
    (*(void (__fastcall **)(_BYTE *))(*(_QWORD *)v5 + 16LL))(v5);
    v6[56] = 0;
  }
  return v6;
}


/* 0x639FF0 */
// Constructs the exact 64-byte common render constant-buffer state.
__int64 __fastcall render_constant_buffer_construct(__int64 a1, __int64 a2, int a3, __int64 a4, __int64 a5)
{
  __int64 result; // rax

  *(_QWORD *)a1 = &unk_192F080;
  *(_QWORD *)(a1 + 8) = *(_QWORD *)a2;
  *(_DWORD *)(a1 + 16) = a3;
  *(_DWORD *)(a1 + 20) = *(_DWORD *)(a2 + 8);
  *(_DWORD *)(a1 + 24) = *(_DWORD *)(a2 + 12);
  result = *(_QWORD *)(a2 + 24);
  *(_QWORD *)(a1 + 32) = result;
  *(_QWORD *)(a1 + 40) = a4;
  *(_QWORD *)(a1 + 48) = a5;
  *(_BYTE *)(a1 + 56) = 1;
  return result;
}


/* 0x63A030 */
// Empty common render constant-buffer destructor.
void render_constant_buffer_destruct()
{
  ;
}


/* 0x63A040 */
// Deleting common render constant-buffer destructor.
// attributes: thunk
double __fastcall render_constant_buffer_delete(__int64 a1)
{
  return sub_37BF50(a1);
}

