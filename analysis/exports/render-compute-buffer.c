/* Generated Hex-Rays evidence for the common render compute-buffer lifecycle. */

/* 0x636C70 */
// Creates a platform compute buffer, allocates its CPU staging block, and initializes backend storage.
_QWORD *__fastcall render_create_compute_buffer(__int64 a1)
{
  _QWORD *v1; // rbx
  __int64 v2; // rdx
  __int64 v3; // rsi

  v1 = (_QWORD *)(*(__int64 (__fastcall **)(_QWORD, __int64))(**(_QWORD **)(g_render_system + 304) + 104LL))(
                   *(_QWORD *)(g_render_system + 304),
                   a1);
  v1[8] = sub_37BF60(v1[3] * v1[2], v3, v2);
  (*(void (__fastcall **)(_QWORD *))(*v1 + 80LL))(v1);
  return v1;
}


/* 0x636CC0 */
// Constructs the 80-byte common compute-buffer state from its 48-byte descriptor.
void *__fastcall render_compute_buffer_construct(_QWORD *a1, __int64 a2)
{
  _R14 = a2;
  _RBX = a1;
  sub_6427D0(a1);
  *_RBX = &unk_192EDA8;
  _RBX[8] = 0;
  __asm
  {
    vmovups ymm0, ymmword ptr [r14]
    vmovups ymm1, ymmword ptr [r14+10h]
    vmovups ymmword ptr [rbx+20h], ymm1
    vmovups ymmword ptr [rbx+10h], ymm0
  }
  return &unk_192EDA8;
}


/* 0x636D10 */
// Releases common compute-buffer CPU staging storage.
__int64 __fastcall render_compute_buffer_destruct(_QWORD *a1, __int64 a2)
{
  __int64 v3; // rdi

  *a1 = &unk_192EDA8;
  v3 = a1[8];
  if ( v3 != 0 )
    sub_37BF70(v3, a2);
  return nullsub_49(a1);
}


/* 0x636D50 */
// Deleting common compute-buffer destructor.
double __fastcall render_compute_buffer_delete(_QWORD *a1, __int64 a2)
{
  __int64 v2; // rax
  __int64 v4; // rdi
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v2;
  *a1 = &unk_192EDA8;
  v4 = a1[8];
  if ( v4 != 0 )
    sub_37BF70(v4, a2);
  nullsub_49(a1);
  return sub_37BF50(a1);
}


/* 0x636DB0 */
// Returns the common compute-buffer type sentinel 0xFFFFFFFF.
__int64 render_compute_buffer_type()
{
  return 0xFFFFFFFFLL;
}

