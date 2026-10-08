/* Generated Hex-Rays evidence for the common render-shader lifecycle. */

/* 0x642250 */
__int64 __fastcall render_create_shader(unsigned int a1)
{
  return (*(__int64 (__fastcall **)(_QWORD, _QWORD))(**(_QWORD **)(g_render_system + 304) + 96LL))(
           *(_QWORD *)(g_render_system + 304),
           a1);
}

/* 0x642270 */
void *__fastcall render_shader_construct(__int64 a1)
{
  *(_QWORD *)a1 = &unk_192F370;
  *(_QWORD *)(a1 + 8) = 0;
  *(_BYTE *)(a1 + 16) = 0;
  *(_QWORD *)(a1 + 24) = -1;
  *(_QWORD *)(a1 + 32) = 0;
  return &unk_192F370;
}

/* 0x6422A0 */
void render_shader_destruct()
{
  ;
}

/* 0x6422B0 */
// attributes: thunk
double __fastcall render_shader_delete(__int64 a1)
{
  return sub_37BF50(a1);
}

/* 0x6422C0 */
__int64 __fastcall render_shader_initialize(_BYTE *a1, __int64 a2, __int64 a3, __int64 a4)
{
  int v6; // eax

  *((_QWORD *)a1 + 1) = a2;
  *((_QWORD *)a1 + 4) = a4;
  if ( a1[16] != 0 )
  {
    (*(void (__fastcall **)(_BYTE *))(*(_QWORD *)a1 + 32LL))(a1);
    a1[16] = 0;
  }
  if ( a3 != 0 )
  {
    v6 = (*(__int64 (__fastcall **)(_BYTE *, __int64))(*(_QWORD *)a1 + 16LL))(a1, a3);
    LODWORD(a4) = v6;
  }
  else
  {
    LOBYTE(a4) = 1;
    LOBYTE(v6) = 0;
  }
  a1[16] = v6;
  return (unsigned int)a4;
}

/* 0x642310 */
__int64 __fastcall render_shader_release(_BYTE *a1)
{
  __int64 result; // rax

  if ( a1[16] != 0 )
  {
    result = (*(__int64 (__fastcall **)(_BYTE *))(*(_QWORD *)a1 + 32LL))(a1);
    a1[16] = 0;
  }
  return result;
}
