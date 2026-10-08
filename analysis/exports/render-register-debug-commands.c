// Registers the renderer debug command table.
__int64 render_register_debug_commands()
{
  __int64 v1; // [rsp+0h] [rbp-D0h] BYREF
  __int64 v2; // [rsp+8h] [rbp-C8h] BYREF
  __int64 v3; // [rsp+10h] [rbp-C0h] BYREF
  __int64 v4; // [rsp+18h] [rbp-B8h] BYREF
  __int64 v5; // [rsp+20h] [rbp-B0h] BYREF
  __int64 v6; // [rsp+28h] [rbp-A8h] BYREF
  __int64 v7; // [rsp+30h] [rbp-A0h] BYREF
  __int64 v8; // [rsp+38h] [rbp-98h] BYREF
  __int64 v9; // [rsp+40h] [rbp-90h] BYREF
  __int64 v10; // [rsp+48h] [rbp-88h] BYREF
  __int64 v11; // [rsp+50h] [rbp-80h] BYREF
  __int64 v12; // [rsp+58h] [rbp-78h] BYREF
  __int64 v13; // [rsp+60h] [rbp-70h] BYREF
  __int64 v14; // [rsp+68h] [rbp-68h] BYREF
  __int64 v15; // [rsp+70h] [rbp-60h] BYREF
  __int64 v16; // [rsp+78h] [rbp-58h] BYREF
  __int64 v17; // [rsp+80h] [rbp-50h] BYREF
  __int64 v18; // [rsp+88h] [rbp-48h] BYREF
  __int64 v19; // [rsp+90h] [rbp-40h] BYREF
  __int64 v20; // [rsp+98h] [rbp-38h] BYREF
  __int64 v21; // [rsp+A0h] [rbp-30h] BYREF
  __int64 v22; // [rsp+A8h] [rbp-28h] BYREF
  __int64 v23; // [rsp+B0h] [rbp-20h] BYREF
  _QWORD v24[3]; // [rsp+B8h] [rbp-18h] BYREF

  v24[1] = 0x6365786562696C2FLL;
  sub_256FD0(v24, "toggle_overlay");
  sub_2221F0(v24[0], render_command_toggle_overlay);
  sub_256FD0(&v23, "overlay_help");
  sub_2221F0(v23, render_command_overlay_help);
  sub_256FD0(&v22, "reload_shaders");
  sub_2221F0(v22, render_command_reload_shaders);
  sub_256FD0(&v21, "set_resolution");
  sub_2221F0(v21, render_command_set_resolution);
  sub_256FD0(&v20, "set_quality_level");
  sub_2221F0(v20, render_command_set_quality_level);
  sub_256FD0(&v19, "toggle_vsync");
  sub_2221F0(v19, render_command_toggle_vsync);
  sub_256FD0(&v18, "toggle_scene_mask");
  sub_2221F0(v18, render_command_toggle_scene_mask);
  sub_256FD0(&v17, "toggle_shadows");
  sub_2221F0(v17, render_command_toggle_shadows);
  sub_256FD0(&v16, "toggle_postproc");
  sub_2221F0(v16, render_command_toggle_postproc);
  sub_256FD0(&v15, "toggle_tonemapping");
  sub_2221F0(v15, render_command_toggle_tonemapping);
  sub_256FD0(&v14, "toggle_vscat");
  sub_2221F0(v14, render_command_toggle_vscat);
  sub_256FD0(&v13, "set_drawn_scene_range");
  sub_2221F0(v13, render_command_set_drawn_scene_range);
  sub_256FD0(&v12, "toggle_multithreaded_rendering");
  sub_2221F0(v12, render_command_toggle_multithreaded_rendering);
  sub_256FD0(&v11, "toggle_async_compute");
  sub_2221F0(v11, render_command_toggle_async_compute);
  sub_256FD0(&v10, "toggle_async_copy");
  sub_2221F0(v10, render_command_toggle_async_copy);
  sub_256FD0(&v9, "toggle_tiled_light_interpolation");
  sub_2221F0(v9, render_command_toggle_tiled_light_interpolation);
  sub_256FD0(&v8, "toggle_partial_framerate");
  sub_2221F0(v8, render_command_toggle_partial_framerate);
  sub_256FD0(&v7, "toggle_stereo_optimizations");
  sub_2221F0(v7, render_command_toggle_stereo_optimizations);
  sub_256FD0(&v6, "toggle_64_bit_light_accum");
  sub_2221F0(v6, render_command_toggle_64_bit_light_accum);
  sub_256FD0(&v5, "toggle_hdr");
  sub_2221F0(v5, render_command_toggle_hdr);
  sub_256FD0(&v4, "take_screenshot");
  sub_2221F0(v4, render_command_take_screenshot);
  sub_256FD0(&v3, "cycle_screenshot_resolution");
  sub_2221F0(v3, render_command_cycle_screenshot_resolution);
  sub_256FD0(&v2, "set_shading_mode");
  sub_2221F0(v2, render_command_set_shading_mode);
  sub_256FD0(&v1, "set_buffer_inspection_mode");
  sub_2221F0(v1, render_command_set_buffer_inspection_mode);
  return 0x6365786562696C2FLL;
}
