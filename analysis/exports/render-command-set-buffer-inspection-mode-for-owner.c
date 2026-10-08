// Renderer debug command handler inferred from the registration table.
__int64 __fastcall render_command_set_buffer_inspection_mode_for_owner(__int64 a1, __int64 a2)
{
  unsigned int v2; // r14d
  int v4; // r15d
  __int64 *v5; // rbx
  int v6; // r13d
  __int64 v7; // rbx
  __int64 i; // rbx
  __int64 v9; // r12
  signed __int32 v10; // eax
  __int64 v12; // [rsp+8h] [rbp-48h]
  __int64 v13; // [rsp+10h] [rbp-40h] BYREF
  int v14; // [rsp+18h] [rbp-38h]
  __int64 v15; // [rsp+20h] [rbp-30h]

  v4 = 0;
  v15 = 0x6365786562696C2FLL;
  if ( *(_WORD *)(a2 + 20) != 2 )
  {
LABEL_23:
    render_debug_view_name(v4);
    render_frame_owner_set_debug_view(a1, v4);
    LOBYTE(v2) = 1;
    goto LABEL_24;
  }
  v5 = &v13;
  sub_C7D30(&v13, a2, 1);
  v12 = a1;
  if ( v14 == 18 )
  {
    v5 = (__int64 *)v13;
  }
  else if ( v14 != 5 )
  {
    v6 = 0;
    if ( v14 != 0 )
      v4 = 0;
    else
      v4 = v13;
LABEL_11:
    if ( (v14 & 0x10) == 0 )
      goto LABEL_20;
    goto LABEL_17;
  }
  v7 = *v5;
  if ( (unsigned int)strcasecmp(v7, "help") == 0 )
  {
    v4 = 0;
    for ( i = 0; i != 74; ++i )
      render_debug_view_name(i);
    LOBYTE(v2) = 1;
  }
  else
  {
    v4 = render_debug_view_from_name(v7);
    v6 = 0;
    if ( v4 != -1 )
      goto LABEL_11;
    v4 = -1;
    v2 = 0;
  }
  v6 = 1;
  if ( (v14 & 0x10) != 0 )
  {
LABEL_17:
    v9 = v13;
    v10 = _InterlockedExchangeAdd((volatile signed __int32 *)(v13 + 16), 0xFFFFFFFF);
    if ( v9 != 0 && v10 == 1 )
    {
      sub_21E500(v9);
      sub_385460(24, v9);
    }
  }
LABEL_20:
  if ( (v14 & 0xFFFFFFFE) == 0x26 )
    sub_2392A0(&v13);
  a1 = v12;
  if ( v6 == 0 )
    goto LABEL_23;
LABEL_24:
  LOBYTE(v2) = v2 & 1;
  return v2;
}
