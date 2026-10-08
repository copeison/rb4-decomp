void __fastcall render_compute_buffer_descriptor_init(__int64 _RDI)
{
  __asm
  {
    vxorps  ymm0, ymm0, ymm0; Zero-initializes the 64-byte compute-buffer descriptor. Descriptive inferred name.
    vmovups ymmword ptr [rdi+10h], ymm0
    vmovups ymmword ptr [rdi], ymm0
  }
}
