// Initializes renderer defaults, loads the rnd config block, applies platform capability gates, and handles the resolution command-line override.
__int64 __fastcall render_settings_initialize(__int64 a1)
{
  __int64 v11; // r13
  __int64 v12; // r14
  _QWORD *v13; // rax
  _QWORD *v14; // rax
  double v15; // xmm0_8
  _QWORD *v16; // rax
  _QWORD *v17; // rax
  double v18; // xmm0_8
  double v19; // xmm0_8
  double v20; // xmm0_8
  double v21; // xmm0_8
  double v22; // xmm0_8
  double v23; // xmm0_8
  double v24; // xmm0_8
  double v25; // xmm0_8
  __int64 v26; // r12
  double v27; // xmm0_8
  double v28; // xmm0_8
  double v29; // xmm0_8
  __int64 v30; // r12
  double v31; // xmm0_8
  __int64 v32; // r12
  double v33; // xmm0_8
  __int64 v34; // r14
  double v35; // xmm0_8
  double v36; // xmm0_8
  double v37; // xmm0_8
  double v38; // xmm0_8
  __int64 v39; // rax
  _DWORD *v40; // rax
  _DWORD *v41; // rcx
  __int64 v42; // rax
  __int64 v44; // [rsp+0h] [rbp-150h] BYREF
  __int64 v45; // [rsp+8h] [rbp-148h] BYREF
  __int64 v46; // [rsp+10h] [rbp-140h] BYREF
  __int64 v47; // [rsp+18h] [rbp-138h] BYREF
  __int64 v48; // [rsp+20h] [rbp-130h] BYREF
  __int64 v49; // [rsp+28h] [rbp-128h] BYREF
  __int64 v50; // [rsp+30h] [rbp-120h] BYREF
  __int64 v51; // [rsp+38h] [rbp-118h] BYREF
  __int64 v52; // [rsp+40h] [rbp-110h] BYREF
  __int64 v53; // [rsp+48h] [rbp-108h] BYREF
  __int64 v54; // [rsp+50h] [rbp-100h] BYREF
  __int64 v55; // [rsp+58h] [rbp-F8h] BYREF
  __int64 v56; // [rsp+60h] [rbp-F0h] BYREF
  __int64 v57; // [rsp+68h] [rbp-E8h] BYREF
  __int64 v58; // [rsp+70h] [rbp-E0h] BYREF
  __int64 v59; // [rsp+78h] [rbp-D8h] BYREF
  __int64 v60; // [rsp+80h] [rbp-D0h] BYREF
  __int64 v61; // [rsp+88h] [rbp-C8h] BYREF
  __int64 v62; // [rsp+90h] [rbp-C0h] BYREF
  __int64 v63; // [rsp+98h] [rbp-B8h] BYREF
  __int64 v64; // [rsp+A0h] [rbp-B0h] BYREF
  __int64 v65; // [rsp+A8h] [rbp-A8h] BYREF
  __int64 v66; // [rsp+B0h] [rbp-A0h] BYREF
  __int64 v67; // [rsp+B8h] [rbp-98h] BYREF
  __int64 v68; // [rsp+C0h] [rbp-90h] BYREF
  __int64 v69; // [rsp+C8h] [rbp-88h] BYREF
  __int64 v70; // [rsp+D0h] [rbp-80h] BYREF
  __int64 v71; // [rsp+D8h] [rbp-78h] BYREF
  __int64 v72; // [rsp+E0h] [rbp-70h] BYREF
  __int64 v73; // [rsp+E8h] [rbp-68h] BYREF
  __int64 v74; // [rsp+F0h] [rbp-60h] BYREF
  __int64 v75; // [rsp+F8h] [rbp-58h] BYREF
  __int64 v76; // [rsp+100h] [rbp-50h] BYREF
  __int64 v77; // [rsp+108h] [rbp-48h] BYREF
  __int64 v78; // [rsp+110h] [rbp-40h] BYREF
  _QWORD v79[7]; // [rsp+118h] [rbp-38h] BYREF

  __asm { vmovups xmm0, cs:xmmword_12B67D0 }
  _RBX = a1;
  __asm { vmovups xmm1, cs:xmmword_12B6800 }
  v79[1] = 0x6365786562696C2FLL;
  __asm
  {
    vmovups xmmword ptr [rbx], xmm0
    vmovups ymm0, cs:ymmword_12B6820
  }
  *(_BYTE *)(a1 + 16) = 0;
  *(_DWORD *)(a1 + 20) = 0;
  *(_BYTE *)(a1 + 24) = 1;
  *(_BYTE *)(a1 + 25) = 1;
  *(_BYTE *)(a1 + 26) = 0;
  *(_BYTE *)(a1 + 27) = 1;
  *(_BYTE *)(a1 + 28) = 0;
  _RAX = -1;
  __asm
  {
    vmovups ymmword ptr [rbx+20h], ymm0
    vmovups xmm0, cs:xmmword_12B67E0
  }
  *(_QWORD *)(a1 + 64) = 256;
  __asm
  {
    vmovups xmmword ptr [rbx+48h], xmm0
    vxorps  xmm0, xmm0, xmm0
  }
  *(_QWORD *)(a1 + 88) = 128;
  *(_BYTE *)(a1 + 96) = 1;
  __asm
  {
    vmovups xmmword ptr [rbx+68h], xmm0
    vmovups xmm0, cs:xmmword_12B67F0
    vmovups xmmword ptr [rbx+78h], xmm0
    vmovq   xmm0, rax
  }
  *(_DWORD *)(a1 + 136) = 1920;
  *(_DWORD *)(a1 + 140) = 1080;
  *(_BYTE *)(a1 + 144) = 0;
  *(_DWORD *)(a1 + 148) = 1;
  *(_WORD *)(a1 + 156) = 257;
  *(_DWORD *)(a1 + 152) = (_DWORD)&loc_1010101;
  __asm
  {
    vpslldq xmm0, xmm0, 8
    vmovdqu xmmword ptr [rbx+0A0h], xmm0
  }
  *(_DWORD *)(a1 + 176) = (_DWORD)&loc_1010101;
  *(_BYTE *)(a1 + 180) = 0;
  *(_BYTE *)(a1 + 181) = 1;
  *(_DWORD *)(a1 + 184) = 0;
  __asm { vmovups xmmword ptr [rbx+0C0h], xmm1 }
  *(_QWORD *)(a1 + 208) = 10;
  *(_WORD *)(a1 + 216) = 0;
  *(_BYTE *)(a1 + 218) = 1;
  *(_WORD *)(a1 + 223) = 0;
  *(_DWORD *)(a1 + 219) = 0;
  v11 = g_render_system;
  sub_256FD0(&v78, byte_12B6840);
  v12 = sub_368B00(v78);
  sub_256FD0(&v77, "content_resolution");
  v13 = (_QWORD *)sub_21C970(v12, v77, 0);
  *(_DWORD *)_RBX = sub_EB40(*v13 + 16LL, v13);
  sub_256FD0(&v76, "content_resolution");
  v14 = (_QWORD *)sub_21C970(v12, v76, 0);
  *(_DWORD *)(_RBX + 4) = sub_EB40(*v14 + 32LL, v14);
  v15 = sub_256FD0(&v75, "pc_init_fullscreen");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 16);
  sub_21D260(v12, v75, v79, 0, v15);
  *(_BYTE *)(_RBX + 16) = v79[0];
  sub_256FD0(&v74, "pc_init_window_resolution");
  v16 = (_QWORD *)sub_21C970(v12, v74, 0);
  *(_DWORD *)(_RBX + 8) = sub_EB40(*v16 + 16LL, v16);
  sub_256FD0(&v73, "pc_init_window_resolution");
  v17 = (_QWORD *)sub_21C970(v12, v73, 0);
  *(_DWORD *)(_RBX + 12) = sub_EB40(*v17 + 32LL, v17);
  sub_256FD0(&v72, "vsync_mode");
  LODWORD(v79[0]) = *(_DWORD *)(_RBX + 20);
  sub_21D060(v12, v72, v79, 0);
  *(_DWORD *)(_RBX + 20) = v79[0];
  v18 = sub_256FD0(&v71, "use_lod");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 24);
  sub_21D260(v12, v71, v79, 0, v18);
  *(_BYTE *)(_RBX + 24) = v79[0];
  v19 = sub_256FD0(&v70, "use_gbuffer_vertex_normals");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 25);
  sub_21D260(v12, v70, v79, 0, v19);
  *(_BYTE *)(_RBX + 25) = v79[0];
  v20 = sub_256FD0(&v69, "use_64_bit_light_accum");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 26);
  sub_21D260(v12, v69, v79, 0, v20);
  *(_BYTE *)(_RBX + 26) = v79[0];
  v21 = sub_256FD0(&v68, "use_40_bit_depth_stencil");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 27);
  sub_21D260(v12, v68, v79, 0, v21);
  *(_BYTE *)(_RBX + 27) = v79[0];
  v22 = sub_256FD0(&v67, "use_tiled_lighting");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 28);
  sub_21D260(v12, v67, v79, 0, v22);
  *(_BYTE *)(_RBX + 28) = v79[0];
  sub_256FD0(&v66, "max_partial_framerate_scenes");
  LODWORD(v79[0]) = *(_DWORD *)(_RBX + 104);
  sub_21D060(v12, v66, v79, 0);
  *(_QWORD *)(_RBX + 104) = SLODWORD(v79[0]);
  sub_256FD0(&v65, "max_shadow_contrib_buffers");
  LODWORD(v79[0]) = *(_DWORD *)(_RBX + 112);
  sub_21D060(v12, v65, v79, 0);
  *(_QWORD *)(_RBX + 112) = SLODWORD(v79[0]);
  *(_BYTE *)(_RBX + 180) = *(_QWORD *)(_RBX + 104) != 0;
  sub_256FD0(&v64, "quality_level");
  sub_256FD0(&v63, 19246190);
  v79[0] = v63;
  sub_21CFC0(v12, v64, v79, 0);
  if ( v79[0] != 19246190 )
    *(_DWORD *)(_RBX + 148) = sub_442540(v79[0]);
  v23 = sub_256FD0(&v62, "scene_mask_enabled");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 153);
  sub_21D260(v12, v62, v79, 0, v23);
  *(_BYTE *)(_RBX + 153) = v79[0];
  v24 = sub_256FD0(&v61, "multi_threaded_rendering_enabled");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 176);
  sub_21D260(v12, v61, v79, 0, v24);
  *(_BYTE *)(_RBX + 176) = v79[0];
  v25 = sub_256FD0(&v60, "async_compute_enabled");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 177);
  sub_21D260(v12, v60, v79, 0, v25);
  *(_BYTE *)(_RBX + 177) = v79[0];
  sub_256FD0(&v59, "max_geo_overdraw");
  LODWORD(v79[0]) = *(_DWORD *)(_RBX + 192);
  sub_21D060(v12, v59, v79, 0);
  *(_QWORD *)(_RBX + 192) = SLODWORD(v79[0]);
  sub_256FD0(&v58, "max_lighting_overdraw");
  LODWORD(v79[0]) = *(_DWORD *)(_RBX + 200);
  sub_21D060(v12, v58, v79, 0);
  *(_QWORD *)(_RBX + 200) = SLODWORD(v79[0]);
  sub_256FD0(&v57, "max_light_probe_overdraw");
  LODWORD(v79[0]) = *(_DWORD *)(_RBX + 208);
  sub_21D060(v12, v57, v79, 0);
  *(_QWORD *)(_RBX + 208) = SLODWORD(v79[0]);
  sub_256FD0(&v56, "graphics_api_validation");
  v26 = sub_21C970(v12, v56, 0);
  v27 = sub_256FD0(&v55, "enabled");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 216);
  sub_21D260(v26, v55, v79, 0, v27);
  *(_BYTE *)(_RBX + 216) = v79[0];
  v28 = sub_256FD0(&v54, "break_on_warning");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 217);
  sub_21D260(v26, v54, v79, 0, v28);
  *(_BYTE *)(_RBX + 217) = v79[0];
  v29 = sub_256FD0(&v53, "break_on_error");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 218);
  sub_21D260(v26, v53, v79, 0, v29);
  *(_BYTE *)(_RBX + 218) = v79[0];
  sub_256FD0(&v52, "graphics_debugger");
  v30 = sub_21C970(v12, v52, 0);
  v31 = sub_256FD0(&v51, "enabled");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 219);
  sub_21D260(v30, v51, v79, 0, v31);
  *(_BYTE *)(_RBX + 219) = v79[0];
  sub_256FD0(&v50, "graphics_barrier_validation");
  v32 = sub_21C970(v12, v50, 0);
  v33 = sub_256FD0(&v49, "enabled");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 220);
  sub_21D260(v32, v49, v79, 0, v33);
  *(_BYTE *)(_RBX + 220) = v79[0];
  sub_256FD0(&v48, "shader_compilation");
  v34 = sub_21C970(v12, v48, 0);
  v35 = sub_256FD0(&v47, "print");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 221);
  sub_21D260(v34, v47, v79, 0, v35);
  *(_BYTE *)(_RBX + 221) = v79[0];
  v36 = sub_256FD0(&v46, "print_verbose");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 222);
  sub_21D260(v34, v46, v79, 0, v36);
  *(_BYTE *)(_RBX + 222) = v79[0];
  v37 = sub_256FD0(&v45, "output_intermediates");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 223);
  sub_21D260(v34, v45, v79, 0, v37);
  *(_BYTE *)(_RBX + 223) = v79[0];
  v38 = sub_256FD0(&v44, "generate_debug_info");
  LOBYTE(v79[0]) = *(_BYTE *)(_RBX + 224);
  sub_21D260(v34, v44, v79, 0, v38);
  *(_BYTE *)(_RBX + 224) = v79[0];
  if ( (*(_BYTE *)(v11 + 1248) & 0x10) != 0 )
  {
    if ( *(_BYTE *)(_RBX + 177) != 0 )
      *(_BYTE *)(_RBX + 176) = 0;
  }
  else
  {
    *(_BYTE *)(_RBX + 177) = 0;
    *(_BYTE *)(_RBX + 28) = 0;
  }
  *(_QWORD *)(_RBX + 136) = *(_QWORD *)(*(_QWORD *)(v11 + 1216) - 8LL);
  v39 = sub_252660(arguments, "resolution", 0);
  if ( v39 != 0 )
  {
    v79[0] = 0;
    if ( (unsigned __int8)sub_441940(v39, v79) != 0 )
    {
      v40 = *(_DWORD **)(v11 + 1208);
      v41 = *(_DWORD **)(v11 + 1216);
      if ( v40 != v41 )
      {
        while ( *v40 != *(_DWORD *)(_RBX + 136) || v40[1] != *(_DWORD *)(_RBX + 140) )
        {
          v40 += 2;
          if ( v41 == v40 )
            return 0x6365786562696C2FLL;
        }
      }
      if ( v40 != v41 )
      {
        v42 = v79[0];
        *(_QWORD *)(_RBX + 136) = v79[0];
        *(_QWORD *)(_RBX + 8) = v42;
        *(_BYTE *)(_RBX + 144) = 1;
      }
    }
  }
  return 0x6365786562696C2FLL;
}
