__int64 __fastcall render_create_default_materials(_QWORD *a1, __int64 a2)
{
  _BYTE *v4; // rbx
  __int64 v5; // rax
  _BYTE *v6; // rbx
  __int64 v7; // rax
  _BYTE *v8; // rbx
  __int64 v9; // rax
  _BYTE *v10; // rbx
  __int64 v11; // rax
  _BYTE *v12; // rbx
  __int64 v13; // rax
  _BYTE *v14; // rbx
  __int64 v15; // rax
  __int64 v17; // [rsp+0h] [rbp-60h] BYREF
  __int64 v18; // [rsp+8h] [rbp-58h] BYREF
  __int64 v19; // [rsp+10h] [rbp-50h] BYREF
  __int64 v20; // [rsp+18h] [rbp-48h] BYREF
  __int64 v21; // [rsp+20h] [rbp-40h] BYREF
  _QWORD v22[7]; // [rsp+28h] [rbp-38h] BYREF

  v22[1] = 0x6365786562696C2FLL;
  v4 = (_BYTE *)sub_F0A10(a2, 0, 0);
  sub_256FD0(v22, "default_mat_unlit");
  sub_1160F0(v4, v22[0]);
  v5 = sub_117700(v4, unk_1A8B928, 0);
  a1[53] = v5;
  rnd_material_set_sharing_type(v5, v4, 2);
  rnd_material_set_shader_graph(a1[53], "../../system/data/shared/shadergraph/default_unlit.sgraph");
  v6 = (_BYTE *)sub_F0A10(a2, 0, 0);
  sub_256FD0(&v21, "default_mat_add");
  sub_1160F0(v6, v21);
  v7 = sub_117700(v6, unk_1A8B928, 0);
  a1[54] = v7;
  rnd_material_set_sharing_type(v7, v6, 2);
  rnd_material_set_blend_mode(a1[54], 6);
  rnd_material_set_shader_graph(a1[54], "../../system/data/shared/shadergraph/default_unlit.sgraph");
  v8 = (_BYTE *)sub_F0A10(a2, 0, 0);
  sub_256FD0(&v20, "default_mat_lit");
  sub_1160F0(v8, v20);
  v9 = sub_117700(v8, unk_1A8B928, 0);
  a1[55] = v9;
  rnd_material_set_sharing_type(v9, v8, 2);
  rnd_material_set_shader_graph(a1[55], "../../system/data/shared/shadergraph/default.sgraph");
  v10 = (_BYTE *)sub_F0A10(a2, 0, 0);
  sub_256FD0(&v19, "default_text_mat");
  sub_1160F0(v10, v19);
  v11 = sub_117700(v10, unk_1A8B928, 0);
  a1[56] = v11;
  rnd_material_set_sharing_type(v11, v10, 2);
  rnd_material_set_shader_graph(a1[56], "../../system/data/shared/shadergraph/default_text_unlit.sgraph");
  v12 = (_BYTE *)sub_F0A10(a2, 0, 0);
  sub_256FD0(&v18, "default_particle_mat");
  sub_1160F0(v12, v18);
  v13 = sub_117700(v12, unk_1A8B928, 0);
  a1[57] = v13;
  rnd_material_set_sharing_type(v13, v12, 2);
  rnd_material_set_blend_mode(a1[57], 6);
  rnd_material_set_shader_graph(a1[57], "../../system/data/shared/shadergraph/default_particle.sgraph");
  v14 = (_BYTE *)sub_F0A10(a2, 0, 0);
  sub_256FD0(&v17, "default_decal_mat");
  sub_1160F0(v14, v17);
  v15 = sub_117700(v14, unk_1A8B928, 0);
  a1[58] = v15;
  rnd_material_set_sharing_type(v15, v14, 2);
  rnd_material_set_blend_mode(a1[58], 5);
  rnd_material_set_shader_graph(a1[58], "../../system/data/shared/shadergraph/default_decal_lit.sgraph");
  return 0x6365786562696C2FLL;
}
