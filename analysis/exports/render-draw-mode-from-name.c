// Finds a material draw-debug mode by its case-sensitive name.
__int64 __fastcall render_draw_mode_from_name(__int64 a1)
{
  _QWORD *v2; // r15
  unsigned __int64 v3; // rbx

  v2 = off_192F7C0;
  v3 = 0;
  while ( (unsigned int)strcmp(*v2, a1) != 0 )
  {
    ++v3;
    ++v2;
    if ( v3 > 0x1F )
    {
      LODWORD(v3) = -1;
      return (unsigned int)v3;
    }
  }
  return (unsigned int)v3;
}
