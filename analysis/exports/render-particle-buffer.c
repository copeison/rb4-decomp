/* Generated Hex-Rays evidence for the common render particle-buffer lifecycle. */

/* 0x6EAFD0 */
// Dispatches particle-buffer creation through the active render system.
__int64 __fastcall render_create_particle_buffer(__int64 a1, __int64 a2)
{
  return (*(__int64 (__fastcall **)(_QWORD, __int64, __int64))(**(_QWORD **)(g_render_system + 304) + 112LL))(
           *(_QWORD *)(g_render_system + 304),
           a1,
           a2);
}


/* 0x6EB000 */
// Constructs the exact 64-byte common particle-buffer state.
void *__fastcall render_particle_buffer_construct(__int64 _RDI, __int64 a2, __int64 a3, __m128 _XMM0)
{
  __asm { vxorps  xmm0, xmm0, xmm0 }
  *(_QWORD *)_RDI = &unk_1939A30;
  *(_QWORD *)(_RDI + 8) = a2;
  __asm { vmovups xmmword ptr [rdi+10h], xmm0 }
  *(_QWORD *)(_RDI + 32) = 0;
  *(_DWORD *)(_RDI + 40) = -1;
  *(_DWORD *)(_RDI + 44) = -1;
  *(_BYTE *)(_RDI + 48) = 0;
  *(_BYTE *)(_RDI + 49) = 1;
  *(_BYTE *)(_RDI + 50) = 0;
  *(_QWORD *)(_RDI + 56) = a3;
  return &unk_1939A30;
}


/* 0x6ECB80 */
// Empty common particle-buffer destructor.
void render_particle_buffer_destruct()
{
  ;
}


/* 0x6ECB90 */
// Deleting common particle-buffer destructor.
// attributes: thunk
double __fastcall render_particle_buffer_delete(__int64 a1)
{
  return sub_37BF50(a1);
}

