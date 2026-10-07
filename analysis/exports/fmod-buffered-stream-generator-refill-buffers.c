__int64 __fastcall fmod_buffered_stream_generator_refill_buffers(__int64 a1)
{
  __int64 result; // rax
  __int64 v3; // r9
  __int64 v4; // rdi
  __int64 v5; // r8
  unsigned __int64 v6; // rsi
  unsigned __int64 v7; // r15
  int v8; // ecx
  int v9; // ebx
  int v10; // r12d
  int v11; // r13d
  __int64 v12; // rax
  int v13; // edx
  int v14; // r12d
  int v15; // esi
  int v16; // r15d
  int v17; // ebx
  int v18; // ecx
  int v19; // edx
  int v20; // ebx
  int v21; // ecx

  if ( *(_BYTE *)(a1 + 492) == 0 )
  {
    v3 = *(_QWORD *)(a1 + 288);
    v4 = *(_QWORD *)(a1 + 280);
    v5 = *(int *)(a1 + 488);
    v6 = 0xAAAAAAAAAAAAAAABLL * ((*(_QWORD *)(a1 + 288) - v4) >> 6);
    v7 = (v5 + (v6 >> 1)) % (int)v6;
    v8 = *(_DWORD *)(a1 + 448);
    v9 = ((int)v5 + 1) % (int)v6;
    v10 = *(_DWORD *)(v4 + 192 * v5 + 136);
    if ( v9 != (_DWORD)v7 )
    {
      v11 = v10 + v8;
      while ( v11 == *(_DWORD *)(v4 + 192LL * v9 + 136) )
      {
        v11 += v8;
        v9 = (v9 + 1) % (int)v6;
        if ( v9 == (_DWORD)v7 )
          goto LABEL_12;
      }
      if ( v9 != (_DWORD)v7 )
      {
        do
        {
          v12 = 192LL * v9;
          v13 = *(_DWORD *)(v4 + v12 + 64);
          if ( v13 != 3 && v13 != 0 )
            break;
          *(_DWORD *)(v4 + v12 + 136) = v11;
          *(_DWORD *)(v4 + 192LL * v9) = v11;
          *(_DWORD *)(v4 + v12 + 4) = *(_DWORD *)(a1 + 448);
          *(_DWORD *)(v4 + v12 + 12) = 0;
          sub_264660(*(_QWORD *)(a1 + 16) + 72LL, v4 + v12);
          v3 = *(_QWORD *)(a1 + 288);
          v4 = *(_QWORD *)(a1 + 280);
          v8 = *(_DWORD *)(a1 + 448);
          v9 = (v9 + 1) % (int)(-1431655765 * ((unsigned __int64)(v3 - v4) >> 6));
          v11 += v8;
        }
        while ( v9 != (_DWORD)v7 );
        v5 = *(int *)(a1 + 488);
        v10 = *(_DWORD *)(v4 + 192 * v5 + 136);
      }
    }
LABEL_12:
    v14 = v10 - v8;
    v15 = -1431655765 * ((unsigned __int64)(v3 - v4) >> 6);
    result = (unsigned int)-v8;
    if ( v14 > (int)result )
    {
      v16 = ((int)v7 - 1) % v15 + (v15 & ((((int)v7 - 1) % v15) >> 31));
      v17 = ((int)v5 - 1) % v15 + (v15 & ((((int)v5 - 1) % v15) >> 31));
      if ( v17 != v16 )
      {
        while ( 1 )
        {
          result = 192LL * v17;
          if ( v14 != *(_DWORD *)(v4 + result + 136) )
            break;
          v14 -= v8;
          result = (unsigned int)((v17 - 1) / v15);
          v17 = (v17 - 1) % v15 + (v15 & (((v17 - 1) % v15) >> 31));
          if ( v17 == v16 )
            return result;
        }
        while ( v17 != v16 )
        {
          if ( v14 <= -v8 )
            break;
          result = 192LL * v17;
          v21 = *(_DWORD *)(v4 + result + 64);
          if ( v21 != 3 && v21 != 0 )
            break;
          *(_DWORD *)(v4 + result + 136) = v14;
          *(_DWORD *)(v4 + 192LL * v17) = v14;
          *(_DWORD *)(v4 + result + 4) = *(_DWORD *)(a1 + 448);
          *(_DWORD *)(v4 + result + 12) = 0;
          sub_264660(*(_QWORD *)(a1 + 16) + 72LL, v4 + result);
          v4 = *(_QWORD *)(a1 + 280);
          v18 = -1431655765 * ((unsigned __int64)(*(_QWORD *)(a1 + 288) - v4) >> 6);
          v19 = (v17 - 1) % v18;
          result = (unsigned int)((v17 - 1) / v18);
          v20 = v18 & (v19 >> 31);
          v8 = *(_DWORD *)(a1 + 448);
          v17 = v19 + v20;
          v14 -= v8;
        }
      }
    }
  }
  return result;
}
