// Allocates an Orbis constant buffer with a 112-byte header and 16 bytes per requested element.
_QWORD *__fastcall orbis_create_constant_buffer(__int64 a1, __int64 a2, unsigned int a3, __int64 a4)
{
  _QWORD *v4; // rbx

  v4 = (_QWORD *)sub_37AE70(16 * a4 + 112, "cbuffer", 0);
  sub_8E3800(v4);
  return v4;
}
