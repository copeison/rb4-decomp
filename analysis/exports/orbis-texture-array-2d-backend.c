/* Generated Hex-Rays evidence for the Orbis 2D texture-array backend. */
/* The initializer at 0x8E5E70 does not currently decompile; see the paired .asm export. */

/* 0x8E5D80 */
void __fastcall orbis_texture_array_2d_destruct(_QWORD *a1)
{
  __int64 v2; // rax
  __int64 v3; // rdi
  __int64 v4; // rdi
  __int64 v5; // rdi

  *a1 = &unk_195FB98;
  orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[45]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[46]);
  v2 = a1[47];
  if ( v2 != 0 )
  {
    orbis_defer_allocation_release(g_orbis_render_system, (unsigned __int64)*(unsigned int *)(v2 + 28) << 8);
    v3 = a1[47];
    if ( v3 != 0 )
      sub_37BF50(v3);
    a1[47] = 0;
  }
  v4 = a1[43];
  if ( v4 != 0 )
    sub_37BF50(v4);
  a1[43] = 0;
  v5 = a1[48];
  if ( v5 != 0 )
    sub_37BF50(v5);
  a1[48] = 0;
  JUMPOUT(0x6984C0);
}

/* 0x8E5E50 */
double __fastcall orbis_texture_array_2d_delete(_QWORD *a1)
{
  __int64 v1; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  orbis_texture_array_2d_destruct(a1);
  return sub_37BF50(a1);
}
