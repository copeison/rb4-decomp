void __fastcall rnd_material_set_blend_mode(__int64 a1, int a2)
{
  if ( *(_DWORD *)(a1 + 224) != a2 )
  {
    *(_DWORD *)(a1 + 224) = a2;
    *(_BYTE *)(a1 + 257) = 1;
  }
}
