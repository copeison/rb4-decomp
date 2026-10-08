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
