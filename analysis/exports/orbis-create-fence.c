// Allocates and constructs a 24-byte Orbis fence.
__int64 __fastcall orbis_create_fence()
{
  __int64 v0; // rax
  __int64 v1; // rbx
  __int64 savedregs; // [rsp+8h] [rbp+0h]

  savedregs = v0;
  v1 = sub_37BF40(24);
  orbis_fence_construct(v1);
  return v1;
}
