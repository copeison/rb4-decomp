// Sets the material draw-debug mode on every render target owned by the frame owner.
__int64 *__fastcall render_frame_owner_set_draw_mode(__int64 a1, int a2)
{
  __int64 *result; // rax
  __int64 v4; // rdx
  __int64 v5; // rcx

  for ( result = (__int64 *)(*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 24LL))(a1);
        v4 != 0;
        *(_DWORD *)(v5 + 16) = a2 )
  {
    v5 = *result++;
    --v4;
  }
  return result;
}
