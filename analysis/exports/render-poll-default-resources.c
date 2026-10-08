__int64 __fastcall render_poll_default_resources(__int64 *a1)
{
  __int64 v2; // rdi
  __int64 result; // rax
  __int64 v4; // rdi

  v2 = *a1;
  if ( v2 != 0 )
    result = sub_FD8B0(v2, *(_QWORD *)(v2 + 48));
  v4 = a1[59];
  if ( v4 != 0 )
    return sub_FD8B0(v4, *(_QWORD *)(v4 + 48));
  return result;
}
