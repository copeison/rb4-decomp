// Atomically snapshots and resets engine, FMOD, rolling buffer-set, and per-source timing percentages.
void __fastcall fmod_audio_consume_timing_report(
        void *state,
        double *engine_average_percent,
        double *engine_maximum_percent,
        double *fmod_average_percent,
        double *fmod_maximum_percent,
        double *reserved_average_1,
        double *reserved_maximum_1,
        double *reserved_average_2,
        double *reserved_maximum_2,
        double *buffer_set_average_percent,
        double *buffer_set_maximum_percent,
        void *source_average_percent,
        void *source_maximum_percent,
        void *source_keys)
{
  char *v16; // r14
  volatile signed __int32 *v17; // rbx
  signed __int32 v18; // eax
  signed __int32 v19; // ett
  __int64 v21; // rsi
  signed __int32 v24; // eax
  signed __int32 v25; // ett
  signed __int32 v26; // eax
  volatile signed __int32 *v27; // rbx
  signed __int32 v28; // ett
  __int64 v31; // rsi
  signed __int32 v33; // eax
  signed __int32 v34; // ett
  signed __int32 v37; // eax
  volatile signed __int32 *v38; // rbx
  signed __int32 v39; // ett
  signed __int32 v42; // eax
  signed __int32 v43; // ett
  volatile signed __int32 *v44; // r15
  volatile signed __int32 *v45; // rbx
  signed __int32 v46; // eax
  signed __int32 v47; // ett
  signed __int32 v50; // eax
  signed __int32 v51; // ett
  unsigned __int64 v52; // rax
  char *v53; // rsi
  char *v54; // rbx
  char *v55; // rcx
  unsigned __int64 v59; // rax
  char *v60; // rsi
  char *v61; // rcx
  _QWORD *v65; // r12
  __int64 v66; // rdx
  __int64 v67; // rsi
  __int64 v68; // rbx
  __int64 v69; // rax
  __int64 v70; // r14
  char *v71; // r12
  _QWORD *v72; // rcx
  __int64 v73; // r12
  char *v74; // [rsp+8h] [rbp-78h]
  char *v75; // [rsp+10h] [rbp-70h]
  char *v76; // [rsp+18h] [rbp-68h]
  __int64 v79; // [rsp+20h] [rbp-60h]
  __int64 v82; // [rsp+28h] [rbp-58h]
  char *v84; // [rsp+30h] [rbp-50h]
  unsigned __int64 v85; // [rsp+38h] [rbp-48h] BYREF
  unsigned __int64 v86; // [rsp+40h] [rbp-40h] BYREF
  _QWORD v87[7]; // [rsp+48h] [rbp-38h] BYREF

  _R12 = fmod_maximum_percent;
  _R13 = engine_average_percent;
  v16 = (char *)state + 384;
  v17 = (volatile signed __int32 *)((char *)state + 424);
  v87[1] = 0x6365786562696C2FLL;
  v18 = _InterlockedCompareExchange((volatile signed __int32 *)state + 106, 1, 0);
  if ( v18 != 0 )
  {
    do
    {
      v19 = v18;
      v18 = _InterlockedCompareExchange(v17, 1, v18);
    }
    while ( v19 != v18 );
  }
  *(double *)&_XMM0 = (*(double (__fastcall **)(char *, double *, double *, __int64, double *, double *))(*((_QWORD *)state + 48) + 24LL))(
                        (char *)state + 384,
                        engine_average_percent,
                        engine_maximum_percent,
                        1,
                        fmod_maximum_percent,
                        reserved_average_1);
  __asm { vmovsd  qword ptr [r13+0], xmm0 }
  *(double *)&_XMM0 = (*(double (__fastcall **)(char *))(*((_QWORD *)state + 48) + 32LL))(v16);
  _RAX = engine_maximum_percent;
  __asm { vmovsd  qword ptr [rax], xmm0 }
  *((_QWORD *)state + 50) = 0;
  *((_DWORD *)state + 102) = 0;
  *((_QWORD *)state + 52) = 0;
  v24 = _InterlockedCompareExchange(v17, 0, 1);
  if ( v24 != 1 )
  {
    do
    {
      v25 = v24;
      v24 = _InterlockedCompareExchange(v17, 0, v24);
    }
    while ( v25 != v24 );
  }
  v26 = _InterlockedCompareExchange((volatile signed __int32 *)state + 124, 1, 0);
  v27 = (volatile signed __int32 *)((char *)state + 496);
  if ( v26 != 0 )
  {
    do
    {
      v28 = v26;
      v26 = _InterlockedCompareExchange(v27, 1, v26);
    }
    while ( v28 != v26 );
  }
  *(double *)&_XMM0 = (*(double (__fastcall **)(char *, __int64, void *, __int64))(*((_QWORD *)state + 57) + 24LL))(
                        (char *)state + 456,
                        v21,
                        state,
                        1);
  _RAX = fmod_average_percent;
  __asm { vmovsd  qword ptr [rax], xmm0 }
  *(double *)&_XMM0 = (*(double (__fastcall **)(char *))(*((_QWORD *)state + 57) + 32LL))((char *)state + 456);
  __asm { vmovsd  qword ptr [r12], xmm0 }
  *((_QWORD *)state + 59) = 0;
  *((_DWORD *)state + 120) = 0;
  *((_QWORD *)state + 61) = 0;
  v33 = _InterlockedCompareExchange(v27, 0, 1);
  if ( v33 != 1 )
  {
    do
    {
      v34 = v33;
      v33 = _InterlockedCompareExchange(v27, 0, v33);
    }
    while ( v34 != v33 );
  }
  _R15 = buffer_set_maximum_percent;
  _R13 = buffer_set_average_percent;
  v37 = _InterlockedCompareExchange((volatile signed __int32 *)state + 142, 1, 0);
  v38 = (volatile signed __int32 *)((char *)state + 568);
  if ( v37 != 0 )
  {
    do
    {
      v39 = v37;
      v37 = _InterlockedCompareExchange(v38, 1, v37);
    }
    while ( v39 != v37 );
  }
  *(double *)&_XMM0 = (*(double (__fastcall **)(char *, __int64, void *, __int64))(*((_QWORD *)state + 66) + 24LL))(
                        (char *)state + 528,
                        v31,
                        state,
                        1);
  __asm { vmovsd  qword ptr [r13+0], xmm0 }
  *(double *)&_XMM0 = (*(double (__fastcall **)(char *))(*((_QWORD *)state + 66) + 32LL))((char *)state + 528);
  __asm { vmovsd  qword ptr [r15], xmm0 }
  *((_QWORD *)state + 68) = 0;
  *((_DWORD *)state + 138) = 0;
  *((_QWORD *)state + 70) = 0;
  v42 = _InterlockedCompareExchange(v38, 0, 1);
  if ( v42 != 1 )
  {
    do
    {
      v43 = v42;
      v42 = _InterlockedCompareExchange(v38, 0, v42);
    }
    while ( v43 != v42 );
  }
  v44 = *((volatile signed __int32 **)state + 79);
  v84 = (char *)state + 632;
  if ( v44 != (volatile signed __int32 *)v84 )
  {
    v76 = (char *)source_keys + 24;
    v75 = (char *)source_average_percent + 8;
    v74 = (char *)source_maximum_percent + 8;
    do
    {
      v45 = v44 + 14;
      v46 = _InterlockedCompareExchange(v44 + 14, 1, 0);
      if ( v46 != 0 )
      {
        do
        {
          v47 = v46;
          v46 = _InterlockedCompareExchange(v45, 1, v46);
        }
        while ( v47 != v46 );
      }
      *(double *)&_XMM0 = (*(double (__fastcall **)(__int64 *))(*((_QWORD *)v44 + 2) + 24LL))((__int64 *)v44 + 2);
      __asm { vmovsd  [rbp+var_58], xmm0 }
      *(double *)&_XMM0 = (*(double (__fastcall **)(__int64 *))(*((_QWORD *)v44 + 2) + 32LL))((__int64 *)v44 + 2);
      v50 = 1;
      __asm { vmovsd  [rbp+var_60], xmm0 }
      *((_QWORD *)v44 + 4) = 0;
      *((_DWORD *)v44 + 10) = 0;
      *((_QWORD *)v44 + 6) = 0;
      do
      {
        v51 = v50;
        v50 = _InterlockedCompareExchange(v45, 0, v50);
      }
      while ( v51 != v50 );
      v52 = *((_QWORD *)v44 + 3);
      v86 = v52;
      v53 = *((char **)source_average_percent + 3);
      if ( v53 != nullptr )
      {
        v54 = (char *)source_average_percent + 8;
        v55 = (char *)source_average_percent + 8;
        while ( 2 )
        {
          _RDX = v53;
          while ( *((_QWORD *)_RDX + 4) < v52 )
          {
            _RDX = *(char **)_RDX;
            if ( _RDX == nullptr )
            {
              _RDX = v55;
              if ( v55 == v75 )
                goto LABEL_31;
              goto LABEL_27;
            }
          }
          v53 = *((char **)_RDX + 1);
          v55 = _RDX;
          if ( v53 != nullptr )
            continue;
          break;
        }
        if ( _RDX == v75 )
          goto LABEL_31;
LABEL_27:
        if ( v52 < *((_QWORD *)_RDX + 4) )
          goto LABEL_32;
      }
      else
      {
        v54 = (char *)source_average_percent + 8;
LABEL_31:
        _RDX = v54;
LABEL_32:
        sub_263CC0(v87, source_average_percent, _RDX, &v86);
        _RDX = (char *)v87[0];
      }
      __asm { vmovsd  xmm0, [rbp+var_58] }
      __asm
      {
        vaddsd  xmm0, xmm0, qword ptr [rdx+28h]
        vmovsd  qword ptr [rdx+28h], xmm0
      }
      v59 = *((_QWORD *)v44 + 3);
      v85 = v59;
      v60 = *((char **)source_maximum_percent + 3);
      if ( v60 != nullptr )
      {
        v61 = (char *)source_maximum_percent + 8;
        while ( 2 )
        {
          _RDX = v60;
          while ( *((_QWORD *)_RDX + 4) < v59 )
          {
            _RDX = *(char **)_RDX;
            if ( _RDX == nullptr )
            {
              _RDX = v61;
              if ( v61 == v74 )
                goto LABEL_44;
              goto LABEL_41;
            }
          }
          v60 = *((char **)_RDX + 1);
          v61 = _RDX;
          if ( v60 != nullptr )
            continue;
          break;
        }
        if ( _RDX == v74 )
          goto LABEL_44;
LABEL_41:
        if ( v59 >= *((_QWORD *)_RDX + 4) )
          goto LABEL_46;
      }
      else
      {
LABEL_44:
        _RDX = (char *)source_maximum_percent + 8;
      }
      sub_263CC0(v87, source_maximum_percent, _RDX, &v85);
      _RDX = (char *)v87[0];
LABEL_46:
      __asm { vmovsd  xmm0, [rbp+var_60] }
      __asm { vaddsd  xmm0, xmm0, qword ptr [rdx+28h] }
      __asm { vmovsd  qword ptr [rdx+28h], xmm0 }
      if ( source_keys != nullptr )
      {
        v65 = *((_QWORD **)source_keys + 1);
        v66 = *((_QWORD *)v44 + 3);
        if ( (unsigned __int64)v65 >= *((_QWORD *)source_keys + 2) )
        {
          v79 = *((_QWORD *)v44 + 3);
          v67 = *(_QWORD *)source_keys;
          v68 = ((__int64)v65 - *(_QWORD *)source_keys) >> 2;
          if ( v65 == *(_QWORD **)source_keys )
            v68 = 1;
          if ( v68 != 0 )
          {
            v69 = sub_252CF0(v76, 8 * v68, 0);
            v67 = *(_QWORD *)source_keys;
            v65 = *((_QWORD **)source_keys + 1);
            v70 = v69;
          }
          else
          {
            v69 = 0;
            v70 = 0;
          }
          v71 = (char *)v65 - v67;
          v82 = v69;
          memmove(v69, v67, v71);
          v72 = source_keys;
          *(_QWORD *)&v71[v70] = v79;
          v73 = (__int64)&v71[v70 + 8];
          if ( *(_QWORD *)source_keys != 0 )
          {
            sub_252D30(v76, *(_QWORD *)source_keys, *((_QWORD *)source_keys + 2) - *(_QWORD *)source_keys);
            v72 = source_keys;
          }
          *v72 = v82;
          v72[1] = v73;
          v72[2] = v70 + 8 * v68;
        }
        else
        {
          *((_QWORD *)source_keys + 1) = v65 + 1;
          *v65 = v66;
        }
      }
      v44 = *(volatile signed __int32 **)v44;
    }
    while ( v44 != (volatile signed __int32 *)v84 );
  }
}
