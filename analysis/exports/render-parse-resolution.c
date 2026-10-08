// Parses WIDTHxHEIGHT or a single 16:9 height into a render extent.
__int64 __fastcall render_parse_resolution(__int64 a1, _DWORD *a2)
{
  unsigned int v3; // ebx
  __int64 v4; // r15
  __int64 v5; // rax
  _QWORD v7[6]; // [rsp+0h] [rbp-30h] BYREF

  v3 = 0;
  v7[1] = 0x6365786562696C2FLL;
  v7[0] = 0;
  v4 = strtol(a1, v7, 0);
  if ( v4 > 0 )
  {
    if ( *(_BYTE *)v7[0] != 120 )
    {
      *a2 = 16 * v4 / 9;
      a2[1] = v4;
      goto LABEL_6;
    }
    v3 = 0;
    v5 = strtol(v7[0] + 1LL, 0, 0);
    if ( v5 > 0 )
    {
      *a2 = v4;
      a2[1] = v5;
LABEL_6:
      LOBYTE(v3) = 1;
    }
  }
  return v3;
}
