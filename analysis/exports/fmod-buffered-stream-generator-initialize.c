__int64 __fastcall fmod_buffered_stream_generator_initialize(__int64 a1, _QWORD *a2, __int64 a3)
{
  int v7; // eax
  _QWORD *v8; // rbx
  _QWORD *v9; // r14
  __int64 v10; // rax
  _QWORD *v11; // rax
  _QWORD **v12; // rcx
  __int64 v13; // rdx
  _QWORD *v14; // rdx
  _QWORD *v15; // rdi
  __int64 v20; // rax
  _QWORD v23[7]; // [rsp+8h] [rbp-38h] BYREF

  _R12 = a1;
  v23[2] = 0x6365786562696C2FLL;
  if ( (unsigned __int8)fmod_buffered_stream_generator_create_sound(a1, a2) != 0 )
  {
    *(_DWORD *)(_R12 + 520) = -1082130432;
    *(_DWORD *)(_R12 + 516) = -1082130432;
    *(_DWORD *)(_R12 + 508) = -1;
    *(_DWORD *)(_R12 + 512) = -1;
    *(_DWORD *)(_R12 + 472) = 1065353216;
    *(_WORD *)(_R12 + 524) = 0;
    fmod_buffered_stream_generator_initialize_buffers(_R12, (_DWORD *)a3);
    if ( fmod_buffered_stream_generator_acquire_bus_generator(_R12, a3) != 0 )
    {
      v7 = *(unsigned __int8 *)(a3 + 16);
      LOBYTE(a3) = 1;
      *(_DWORD *)(_R12 + 28) = v7 + 3;
    }
    else
    {
      v8 = *(_QWORD **)(_R12 + 280);
      v9 = *(_QWORD **)(_R12 + 288);
      if ( v8 != v9 )
      {
        do
        {
          v10 = v8[23];
          if ( v10 != 0 )
          {
            v8[23] = 0;
            --*(_QWORD *)(v10 + 16);
            v11 = v8 + 21;
            v12 = (_QWORD **)(v8 + 22);
            v13 = v8[21];
            *(_QWORD *)(v13 + 8) = v8[22];
            *(_QWORD *)v8[22] = v13;
            v14 = v8 + 21;
            v8[21] = v8 + 21;
            v8[22] = v8 + 21;
          }
          else
          {
            v11 = (_QWORD *)v8[21];
            v14 = (_QWORD *)v8[22];
            v12 = (_QWORD **)(v8 + 22);
          }
          v11[1] = v14;
          **v12 = v11;
          v15 = (_QWORD *)v8[14];
          if ( v15 != nullptr )
          {
            (*(void (__fastcall **)(_QWORD *, bool))(*v15 + 32LL))(v15, v8 + 10 != v15);
            v8[14] = 0;
          }
          v8 += 24;
        }
        while ( v9 != v8 );
        v8 = *(_QWORD **)(_R12 + 280);
      }
      *(_QWORD *)(_R12 + 288) = v8;
      _RBX = (_QWORD *)(_R12 + 344);
      if ( *(_DWORD *)(_R12 + 336) == 0 )
      {
        sub_37B800(*_RBX - 2LL, *(double *)&_XMM0);
        *_RBX = 0;
      }
      __asm { vxorps  ymm0, ymm0, ymm0 }
      LODWORD(a3) = 0;
      __asm
      {
        vmovups ymmword ptr [rbx+20h], ymm0
        vmovups ymmword ptr [rbx], ymm0
        vxorps  xmm0, xmm0, xmm0
      }
      *(double *)&_XMM0 = sub_D3D20(v23, 0, 0, 0, *(double *)&_XMM0);
      v20 = v23[0];
      __asm { vxorps  xmm0, xmm0, xmm0 }
      *(_QWORD *)(_R12 + 413) = *(_QWORD *)((char *)v23 + 5);
      *(_QWORD *)(_R12 + 408) = v20;
      *(_DWORD *)(_R12 + 336) = 1;
      __asm { vmovups xmmword ptr [r12+1A8h], xmm0 }
      *(_DWORD *)(_R12 + 440) = 0;
      *(_BYTE *)(_R12 + 444) = 1;
      FMOD::Sound::release(*(FMOD::Sound **)(_R12 + 456));
    }
  }
  else
  {
    LODWORD(a3) = 0;
  }
  return (unsigned int)a3;
}
