// Constructs an Orbis fence with a zeroed four-byte GPU allocation named PS4Fence.
_DWORD *__fastcall orbis_fence_construct(__int64 a1)
{
  _DWORD *result; // rax

  *(_QWORD *)a1 = &unk_195F6E8;
  *(_QWORD *)(a1 + 8) = 0;
  *(_DWORD *)(a1 + 16) = 0;
  result = (_DWORD *)sub_37AE70(4, "PS4Fence", 4);
  *(_QWORD *)(a1 + 8) = result;
  *result = 0;
  return result;
}
