// Renderer debug command handler inferred from the registration table.
__int64 __fastcall render_command_set_shading_mode_for_owner(__int64 a1, __int64 a2)
{
  unsigned int v2; // r14d
  unsigned int v4; // r15d
  __int64 *v5; // rbx
  int v6; // r13d
  __int64 v7; // rbx
  __int64 v8; // r12
  signed __int32 v9; // eax
  __int64 v11; // [rsp+8h] [rbp-48h]
  __int64 v12; // [rsp+10h] [rbp-40h] BYREF
  int v13; // [rsp+18h] [rbp-38h]
  __int64 v14; // [rsp+20h] [rbp-30h]

  v4 = 0;
  v14 = 0x6365786562696C2FLL;
  if ( *(_WORD *)(a2 + 20) != 2 )
  {
LABEL_21:
    render_draw_mode_name(v4);
    render_frame_owner_set_draw_mode(a1, v4);
    LOBYTE(v2) = 1;
    goto LABEL_22;
  }
  v5 = &v12;
  sub_C7D30(&v12, a2, 1);
  v11 = a1;
  if ( v13 == 18 )
  {
    v5 = (__int64 *)v12;
  }
  else if ( v13 != 5 )
  {
    v6 = 0;
    if ( v13 != 0 )
      v4 = 0;
    else
      v4 = v12;
LABEL_11:
    if ( (v13 & 0x10) == 0 )
      goto LABEL_18;
    goto LABEL_15;
  }
  v7 = *v5;
  if ( (unsigned int)strcasecmp(v7, "help") == 0 )
  {
    v4 = 0;
    render_draw_mode_name(0);
    render_draw_mode_name(1u);
    render_draw_mode_name(2u);
    render_draw_mode_name(3u);
    render_draw_mode_name(4u);
    render_draw_mode_name(5u);
    render_draw_mode_name(6u);
    render_draw_mode_name(7u);
    render_draw_mode_name(8u);
    render_draw_mode_name(9u);
    render_draw_mode_name(0xAu);
    render_draw_mode_name(0xBu);
    render_draw_mode_name(0xCu);
    render_draw_mode_name(0xDu);
    render_draw_mode_name(0xEu);
    render_draw_mode_name(0xFu);
    render_draw_mode_name(0x10u);
    render_draw_mode_name(0x11u);
    render_draw_mode_name(0x12u);
    render_draw_mode_name(0x13u);
    render_draw_mode_name(0x14u);
    render_draw_mode_name(0x15u);
    render_draw_mode_name(0x16u);
    render_draw_mode_name(0x17u);
    render_draw_mode_name(0x18u);
    render_draw_mode_name(0x19u);
    render_draw_mode_name(0x1Au);
    render_draw_mode_name(0x1Bu);
    render_draw_mode_name(0x1Cu);
    render_draw_mode_name(0x1Du);
    render_draw_mode_name(0x1Eu);
    render_draw_mode_name(0x1Fu);
    LOBYTE(v2) = 1;
  }
  else
  {
    v4 = render_draw_mode_from_name(v7);
    v6 = 0;
    if ( v4 != -1 )
      goto LABEL_11;
    v4 = -1;
    v2 = 0;
  }
  v6 = 1;
  if ( (v13 & 0x10) != 0 )
  {
LABEL_15:
    v8 = v12;
    v9 = _InterlockedExchangeAdd((volatile signed __int32 *)(v12 + 16), 0xFFFFFFFF);
    if ( v8 != 0 && v9 == 1 )
    {
      sub_21E500(v8);
      sub_385460(24, v8);
    }
  }
LABEL_18:
  if ( (v13 & 0xFFFFFFFE) == 0x26 )
    sub_2392A0(&v12);
  a1 = v11;
  if ( v6 == 0 )
    goto LABEL_21;
LABEL_22:
  LOBYTE(v2) = v2 & 1;
  return v2;
}
