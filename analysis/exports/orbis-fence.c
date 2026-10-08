/* Generated Hex-Rays evidence for the Orbis GPU fence lifecycle and commands. */

/* 0x8D85C0 */
// Allocates and constructs a 24-byte Orbis fence.
__int64 orbis_create_fence()
{
  __int64 v0; // rbx

  v0 = sub_37BF40(24);
  orbis_fence_construct(v0);
  return v0;
}


/* 0x8E1570 */
// Constructs an Orbis fence with a zeroed four-byte GPU allocation named PS4Fence.
_DWORD *__fastcall orbis_fence_construct(__int64 a1)
{
  _DWORD *result; // rax

  *(_QWORD *)a1 = &unk_195F6E8;
  *(_QWORD *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 16) = 0;
  result = (_DWORD *)sub_37AE70(4, "PS4Fence", 4);
  *(_QWORD *)(a1 + 8) = result;
  *result = 0;
  return result;
}


/* 0x8E15F0 */
void __fastcall orbis_fence_destruct(_QWORD *a1, double a2)
{
  __int64 v3; // rsi

  *a1 = &unk_195F6E8;
  v3 = a1[1];
  if ( g_orbis_render_system != 0 )
    orbis_defer_allocation_release(g_orbis_render_system, v3);
  else
    sub_37B800(v3, a2);
  a1[1] = 0;
}


/* 0x8E1640 */
void __fastcall orbis_fence_base_destruct(__int64 a1, double a2)
{
  __int64 v3; // rsi

  v3 = *(_QWORD *)(a1 + 8);
  if ( g_orbis_render_system != 0 )
    orbis_defer_allocation_release(g_orbis_render_system, v3);
  else
    sub_37B800(v3, a2);
  *(_QWORD *)(a1 + 8) = 0;
}


/* 0x8E1680 */
double __fastcall orbis_fence_delete(_QWORD *a1, double a2)
{
  __int64 v3; // rsi

  *a1 = &unk_195F6E8;
  v3 = a1[1];
  if ( g_orbis_render_system != 0 )
    orbis_defer_allocation_release(g_orbis_render_system, v3);
  else
    sub_37B800(v3, a2);
  return sub_37BF50(a1);
}


/* 0x8E16D0 */
__int64 __fastcall orbis_fence_next_value(__int64 a1, double a2)
{
  int v3; // eax
  __int64 v4; // rsi
  _DWORD *v5; // rax
  __int64 result; // rax

  v3 = *(_DWORD *)(a1 + 16);
  if ( v3 == -1 )
  {
    *(_DWORD *)(a1 + 16) = 0;
    v4 = *(_QWORD *)(a1 + 8);
    if ( g_orbis_render_system != 0 )
      orbis_defer_allocation_release(g_orbis_render_system, v4);
    else
      sub_37B800(v4, a2);
    *(_QWORD *)(a1 + 8) = 0;
    v5 = (_DWORD *)sub_37AE70(4, "PS4Fence", 4);
    *(_QWORD *)(a1 + 8) = v5;
    *v5 = 0;
    v3 = *(_DWORD *)(a1 + 16);
  }
  result = (unsigned int)(v3 + 1);
  *(_DWORD *)(a1 + 16) = result;
  return result;
}


/* 0x8EB730 */
__int64 __fastcall orbis_render_context_signal_fence(__int64 a1, __int64 a2, double a3)
{
  __int64 result; // rax
  __int64 v5; // r15
  __int64 v6; // r14
  unsigned int v7; // eax
  __int64 v8; // r15
  __int64 v9; // r12
  __int64 v10; // r14
  unsigned int value; // eax

  result = *(unsigned int *)(a1 + 18980);
  if ( (_DWORD)result == 1 )
  {
    v8 = 6880LL * *(_QWORD *)(a1 + 18984);
    v9 = 61920LL * *(_QWORD *)(a1 + 265616);
    v10 = *(_QWORD *)(a2 + 8);
    value = orbis_fence_next_value(a2, a3);
    return gnm_compute_command_buffer_release_memory(v8 + a1 + v9 + 141792, 47, 0, v10, 1, value, 0, 0);
  }
  else if ( (_DWORD)result == 0 )
  {
    v5 = 59528LL * *(_QWORD *)(a1 + 265616);
    v6 = *(_QWORD *)(a2 + 8);
    v7 = orbis_fence_next_value(a2, a3);
    return gnm_draw_command_buffer_write_release_mem_event(a1 + v5 + 22312, 40, 0, v6, 1, v7, 0, 0);
  }
  return result;
}


/* 0x8EB7F0 */
__int64 __fastcall orbis_render_context_wait_fence(__int64 a1, __int64 a2)
{
  __int64 result; // rax

  result = *(unsigned int *)(a1 + 18980);
  if ( (_DWORD)result == 1 )
    return gnm_compute_command_buffer_wait_on_address(
             6880LL * *(_QWORD *)(a1 + 18984) + a1 + 61920LL * *(_QWORD *)(a1 + 265616) + 141792,
             *(_QWORD *)(a2 + 8),
             0xFFFFFFFF,
             5u,
             *(_DWORD *)(a2 + 16));
  if ( (_DWORD)result == 0 )
    return (__int64)gnm_draw_command_buffer_wait_on_address(
                      a1 + 59528LL * *(_QWORD *)(a1 + 265616) + 22312,
                      *(_QWORD *)(a2 + 8),
                      -1,
                      5,
                      *(_DWORD *)(a2 + 16));
  return result;
}
