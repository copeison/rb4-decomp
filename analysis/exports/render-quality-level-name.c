// Returns Low, Medium, or High for renderer quality levels 0 through 2.
__int64 __fastcall render_quality_level_name(unsigned int a1)
{
  if ( a1 >= 3 )
    return 19246190;
  else
    return off_1901E30[a1];
}
