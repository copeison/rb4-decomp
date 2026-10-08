/* Generated Hex-Rays evidence for the Orbis particle-buffer backend. */

/* 0x8E2D30 */
// Orbis particle-buffer destructor; defers release of both vertex banks and the index buffer.
double __fastcall orbis_particle_buffer_destruct(_QWORD *a1)
{
  *a1 = &unk_195F790;
  orbis_defer_allocation_release(g_orbis_render_system, a1[40]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[41]);
  return orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
}


/* 0x8E2D80 */
// Deleting Orbis particle-buffer destructor; releases all three GPU allocations before object deletion.
double __fastcall orbis_particle_buffer_delete(_QWORD *a1)
{
  *a1 = &unk_195F790;
  orbis_defer_allocation_release(g_orbis_render_system, a1[40]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[41]);
  orbis_defer_allocation_release(g_orbis_render_system, a1[44]);
  return sub_37BF50(a1);
}


/* 0x8E2DE0 */
// Flips the active particle vertex bank and regenerates its 208-byte quad vertices.
__int64 __fastcall orbis_particle_buffer_upload_vertices(__int64 a1, __int64 a2)
{
  _BOOL8 v2; // rax

  v2 = (*(_DWORD *)(a1 + 336) & 1) == 0;
  *(_QWORD *)(a1 + 336) = v2;
  return particle_buffer_generate_vertices(a1, a2, a2 + 144, *(_QWORD *)(a1 + 8 * v2 + 320));
}


/* 0x8E2E10 */
// Draws active particles as indexed triangle-list quads after binding particle and instance streams.
// bad sp value at call has been detected, the output may be wrong!
__int64 __fastcall orbis_particle_buffer_draw(__int64 a1, __int64 a2, __int64 *a3)
{
  _BOOL8 v6; // rax
  __int64 v7; // r13
  __int64 i; // rbx
  int v9; // eax
  _QWORD *v10; // r8
  unsigned int v14; // r13d
  __int64 v17; // rcx
  __int64 v18; // rbx
  __int64 *v19; // r15
  __int64 v23; // rax
  __int64 v24; // r13
  _QWORD *v25; // r15
  __int64 v26; // rbx
  int v28; // [rsp+Ch] [rbp-144h]
  int v29; // [rsp+10h] [rbp-140h]
  int v30; // [rsp+14h] [rbp-13Ch]
  unsigned int v31; // [rsp+18h] [rbp-138h]
  int v32; // [rsp+1Ch] [rbp-134h]
  unsigned int v33; // [rsp+20h] [rbp-130h]
  int v34; // [rsp+24h] [rbp-12Ch]
  int v35; // [rsp+28h] [rbp-128h]
  int v36; // [rsp+2Ch] [rbp-124h]
  int v37; // [rsp+30h] [rbp-120h]
  int v38; // [rsp+34h] [rbp-11Ch]
  int v39; // [rsp+38h] [rbp-118h]
  int v40; // [rsp+3Ch] [rbp-114h]
  int v41; // [rsp+40h] [rbp-110h]
  int v42; // [rsp+44h] [rbp-10Ch]
  unsigned int v43; // [rsp+48h] [rbp-108h]
  int v44; // [rsp+4Ch] [rbp-104h]
  int v45; // [rsp+50h] [rbp-100h]
  unsigned int v46; // [rsp+54h] [rbp-FCh]
  int v47; // [rsp+58h] [rbp-F8h]
  unsigned int v48; // [rsp+58h] [rbp-F8h]
  _BYTE v50[20]; // [rsp+78h] [rbp-D8h]
  _QWORD v51[24]; // [rsp+90h] [rbp-C0h] BYREF

  v51[18] = 0x6365786562696C2FLL;
  v6 = (*(_DWORD *)(a1 + 336) & 1) == 0;
  *(_QWORD *)(a1 + 336) = v6;
  particle_buffer_generate_vertices(a1, a2, a2 + 144, *(_QWORD *)(a1 + 8 * v6 + 320));
  if ( *(_QWORD *)(a1 + 16) != 0 )
  {
    v7 = a1 + 64;
    for ( i = 0; i != 8; ++i )
    {
      v9 = *(_DWORD *)(a1 + 344);
      if ( _bittest(&v9, i) )
        v10 = (_QWORD *)(v7 + (*(_QWORD *)(a1 + 336) << 7));
      else
        v10 = (_QWORD *)(g_orbis_render_system + 16LL * (int)i + 3852);
      gnmx_cue_set_vertex_buffers(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 2u, i, 1u, v10);
      v7 += 16;
    }
    _RAX = &unk_1B5D268;
    *(_DWORD *)v50 = 0;
    __asm
    {
      vmovups xmm0, xmmword ptr [rax]
      vmovups xmmword ptr [rbp-0D4h], xmm0
      vmovups xmm0, xmmword ptr [rax]
    }
    __asm { vmovups xmmword ptr [rbp-0F0h], xmm0 }
    v14 = dword_19E668C[1];
    v30 = unk_19E6670;
    v28 = unk_19E6674;
    v33 = dword_19E666C;
    v35 = unk_19E6678;
    v32 = unk_19E667C;
    v29 = unk_19E6680;
    v36 = unk_19E6684;
    v34 = unk_19E6688;
    v31 = dword_19E668C[0];
    _RCX = *a3;
    __asm
    {
      vmovups xmm0, xmmword ptr [rcx+58h]
      vmovups xmmword ptr [rbp-0D4h], xmm0
    }
    v17 = 59528LL * *(_QWORD *)(a2 + 265616);
    v18 = *(_QWORD *)(a2 + v17 + 22320);
    v19 = (__int64 *)(a2 + v17 + 22320);
    v43 = dword_19E668C[3];
    v46 = dword_19E668C[2];
    v39 = unk_19E65E4;
    v45 = unk_19E65EC;
    v44 = unk_19E65E0;
    v42 = unk_19E65F8;
    v41 = unk_19E65F0;
    v40 = unk_19E65FC;
    v38 = unk_19E65E8;
    v37 = unk_19E65F4;
    v47 = unk_19E6600;
    if ( (unsigned int)((unsigned __int64)(v18 - *(_QWORD *)(a2 + v17 + 22328)) >> 2) <= 0x1F )
    {
      if ( (*(unsigned __int8 (__fastcall **)(__int64, __int64, _QWORD))(a2 + v17 + 22336))(
             a2 + v17 + 22312,
             32,
             *(_QWORD *)(a2 + v17 + 22344)) == 0 )
      {
        _RBX = nullptr;
        goto LABEL_12;
      }
      v18 = *v19;
    }
    _RBX = (_DWORD *)((v18 - 120) & 0xFFFFFFFFFFFFFFFCLL);
    *v19 = (__int64)_RBX;
LABEL_12:
    orbis_build_instance_vertex_descriptors((__int64)v51, (__int64)_RBX, 1);
    *_RBX = v33;
    _RBX[1] = v35;
    _RBX[2] = v36;
    _RBX[3] = v14;
    _RBX[4] = v30;
    _RBX[5] = v32;
    _RBX[6] = v34;
    _RBX[7] = v46;
    _RBX[8] = v28;
    _RBX[9] = v29;
    _RBX[10] = v31;
    _RBX[11] = v43;
    _RBX[12] = v44;
    _RBX[13] = v45;
    _RBX[14] = v42;
    _RBX[15] = v39;
    _RBX[16] = v41;
    _RBX[17] = v40;
    _RBX[18] = v38;
    _RBX[19] = v37;
    _RBX[20] = v47;
    _RBX[25] = *(_DWORD *)&v50[16];
    __asm
    {
      vmovups xmm0, xmmword ptr [rbp-0D8h]
      vmovups xmmword ptr [rbx+54h], xmm0
      vmovups xmm0, xmmword ptr [rbp-0F0h]
      vmovups xmmword ptr [rbx+68h], xmm0
    }
    gnmx_cue_set_vertex_buffers(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 23056, 2u, 8u, 9u, v51);
    orbis_set_primitive_type(a2, 3u);
    v48 = 6 * *(_DWORD *)(a1 + 16);
    gnm_draw_command_buffer_set_index_size(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312, 0, 2);
    v23 = 59528LL * *(_QWORD *)(a2 + 265616);
    v24 = *(_QWORD *)(a1 + 352);
    v25 = (_QWORD *)(a2 + v23 + 23056);
    v26 = a2 + v23 + 22312;
    gnmx_prepare_draw(v25);
    gnm_draw_command_buffer_draw_index(v26, v48, v24, 0);
    gnmx_finish_draw((__int64)v25);
    nullsub_53(a1, a2);
  }
  return 0x6365786562696C2FLL;
}

