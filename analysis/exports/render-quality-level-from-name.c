// Parses Low, Medium, or High case-insensitively; returns -1 on failure.
char __fastcall render_quality_level_from_name(__int64 a1)
{
  int v2; // eax

  if ( (unsigned int)strcasecmp("Low", a1) == 0 )
  {
    LOBYTE(v2) = 0;
  }
  else if ( (unsigned int)strcasecmp("Medium", a1) == 0 )
  {
    LOBYTE(v2) = 1;
  }
  else
  {
    return (unsigned int)strcasecmp("High", a1) != 0 ? -1 : 2;
  }
  return v2;
}
