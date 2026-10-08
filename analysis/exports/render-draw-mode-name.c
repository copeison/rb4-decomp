// Returns the name for a material draw-debug mode.
__int64 __fastcall render_draw_mode_name(unsigned int a1)
{
  if ( a1 >= 0x20 )
    return 19246190;
  else
    return off_192F7C0[a1];
}
