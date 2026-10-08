// Runs the Orbis render-system destructor and frees its allocation.
__int64 __fastcall orbis_render_system_delete(__int64 a1)
{
  __int64 v1; // rax
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v1;
  orbis_render_system_destruct(a1);
  return sub_37BF50(a1);
}
