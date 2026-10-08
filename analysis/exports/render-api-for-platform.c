// Reads the configured render API name for a platform and converts it to the seven-value API enum.
__int64 __fastcall render_api_for_platform(__int64 a1)
{
  __int64 v1; // rbx
  double v2; // xmm0_8
  _QWORD *v3; // rax
  __int64 v4; // r14
  unsigned int v5; // ebx
  __int64 v7; // [rsp+8h] [rbp-38h] BYREF
  _QWORD v8[6]; // [rsp+10h] [rbp-30h] BYREF

  v8[1] = 0x6365786562696C2FLL;
  v1 = render_platform_name(a1);
  sub_256FD0(v8, byte_1281F90);
  v2 = sub_256FD0(&v7, &dword_1281F94);
  v3 = (_QWORD *)sub_369A90(v8[0], v1, v7, v2);
  v4 = sub_E850(*v3 + 16LL, v3);
  v5 = 0;
  if ( render_api_name(0) != v4 )
  {
    v5 = 1;
    if ( render_api_name(1u) != v4 )
    {
      v5 = 2;
      if ( render_api_name(2u) != v4 )
      {
        v5 = 3;
        if ( render_api_name(3u) != v4 )
        {
          v5 = 4;
          if ( render_api_name(4u) != v4 )
          {
            v5 = 5;
            if ( render_api_name(5u) != v4 )
            {
              v5 = 0;
              if ( render_api_name(6u) == v4 )
                return 6;
            }
          }
        }
      }
    }
  }
  return v5;
}
