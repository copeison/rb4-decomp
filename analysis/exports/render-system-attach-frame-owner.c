// Activates a frame owner, validates its output size, and collects its transient render objects.
char __fastcall render_system_attach_frame_owner(_QWORD *a1, __int64 a2)
{
  unsigned __int64 v4; // rax
  __int64 v5; // rdi
  __int64 v6; // rcx
  __int64 v7; // rax
  unsigned __int64 v8; // rdx
  __int64 v9; // r15
  __m128 v10; // xmm0
  __int64 v11; // rax
  unsigned __int64 v12; // r12
  unsigned __int64 v13; // rcx
  __int64 v15; // rax

  scePthreadSelf(a1);
  a1[15] = a2;
  (*(void (__fastcall **)(__int64))(*(_QWORD *)a2 + 40LL))(a2);
  v4 = sub_448730(a1[15]);
  if ( HIDWORD(v4) == 0 || (_DWORD)v4 == 0 )
  {
    scePthreadSelf(v5);
    a1[17] = a1[16];
    a1[15] = 0;
    return 0;
  }
  v6 = a1[24];
  v7 = a1[23];
  a1[24] = v6 + 1;
  *(_QWORD *)(v7 + 8 * v6) = a1[15];
  v9 = (*(__int64 (__fastcall **)(_QWORD))(*(_QWORD *)a1[15] + 24LL))(a1[15]);
  v11 = a1[16];
  v12 = v8;
  v13 = (a1[17] - v11) >> 3;
  if ( v8 > v13 )
  {
    sub_3DF000(a1 + 16, v8 - v13);
    goto LABEL_7;
  }
  a1[17] = v11 + 8 * v8;
  if ( v8 != 0 )
  {
LABEL_7:
    v15 = 0;
    do
    {
      *(_QWORD *)(a1[16] + 8 * v15) = *(_QWORD *)(v9 + 8 * v15);
      ++v15;
    }
    while ( v12 != v15 );
  }
  sub_6BC3B0(a1[7], 0, v10);
  return 1;
}
