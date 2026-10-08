// Finds a buffer debug view by its case-insensitive name.
__int64 __fastcall render_debug_view_from_name(__int64 a1)
{
  _QWORD *v2; // rbx
  unsigned __int64 v3; // r15

  v2 = off_1936E20;
  v3 = 0;
  while ( (unsigned int)strcasecmp(a1, *v2) != 0 )
  {
    ++v3;
    ++v2;
    if ( v3 > 0x49 )
    {
      LODWORD(v3) = -1;
      return (unsigned int)v3;
    }
  }
  return (unsigned int)v3;
}
