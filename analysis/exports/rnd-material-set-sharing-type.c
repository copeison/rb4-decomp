void __fastcall rnd_material_set_sharing_type(__int64 a1, __int64 a2, int a3)
{
  if ( *(_DWORD *)(a1 + 216) != a3 )
  {
    *(_DWORD *)(a1 + 216) = a3;
    *(_BYTE *)(a1 + 256) = 0;
    if ( a3 == 0 )
      sub_50F2D0(a2);
    JUMPOUT(0x4F3910);
  }
}
