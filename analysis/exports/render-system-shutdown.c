// Flushes deferred releases and shuts down renderer resources, frame owners, and the backend.
__int64 __fastcall render_system_shutdown(__int64 a1)
{
  __int64 v2; // r14
  int v3; // eax
  __int64 *v4; // rbx
  __m128 v5; // xmm0
  __int64 v6; // r14
  __int64 v7; // r14
  __int64 v8; // rax
  unsigned __int64 v9; // rbx

  v2 = a1 + 3752;
  *(_BYTE *)(a1 + 177) = 1;
  scePthreadMutexLock(a1 + 3752);
  v3 = *(_DWORD *)(a1 + 3744) + 1;
  *(_DWORD *)(a1 + 3744) = v3;
  v4 = *(__int64 **)(a1 + 3760);
  if ( v4 != *(__int64 **)(a1 + 3768) )
  {
    do
      sub_4F7270(*v4++);
    while ( v4 != *(__int64 **)(a1 + 3768) );
    v4 = *(__int64 **)(a1 + 3760);
    v3 = *(_DWORD *)(a1 + 3744);
  }
  *(_QWORD *)(a1 + 3768) = v4;
  *(_DWORD *)(a1 + 3744) = v3 - 1;
  scePthreadMutexUnlock(v2);
  render_release_default_resources((_QWORD *)(a1 + 1976));
  sub_451CC0((__int64 *)(a1 + 3560));
  sub_47F580((__int64 *)(a1 + 3256), v5);
  sub_641740(a1 + 2544);
  v6 = *(_QWORD *)(a1 + 3568);
  if ( v6 != 0 )
  {
    sub_460880(*(_QWORD *)(a1 + 3568));
    sub_37BF50(v6);
  }
  *(_QWORD *)(a1 + 3568) = 0;
  v7 = *(_QWORD *)(a1 + 3576);
  if ( v7 != 0 )
  {
    sub_4576A0(*(_QWORD *)(a1 + 3576));
    sub_37BF50(v7);
  }
  *(_QWORD *)(a1 + 3576) = 0;
  sub_639FC0((_QWORD *)(a1 + 3712));
  sub_639FC0((_QWORD *)(a1 + 3720));
  sub_639FC0((_QWORD *)(a1 + 3728));
  sub_639FC0((_QWORD *)(a1 + 3736));
  (*(void (__fastcall **)(_QWORD))(**(_QWORD **)(a1 + 56) + 24LL))(*(_QWORD *)(a1 + 56));
  v8 = *(_QWORD *)(a1 + 72);
  if ( *(_QWORD *)(a1 + 80) != v8 )
  {
    v9 = 0;
    do
    {
      (*(void (__fastcall **)(_QWORD))(**(_QWORD **)(v8 + 8 * v9) + 24LL))(*(_QWORD *)(v8 + 8 * v9));
      v8 = *(_QWORD *)(a1 + 72);
      ++v9;
    }
    while ( v9 < (*(_QWORD *)(a1 + 80) - v8) >> 3 );
  }
  return (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)a1 + 40LL))(a1);
}
