/* Generated Hex-Rays evidence for the Orbis occlusion-query backend. */

/* 0x8E28F0 */
// Orbis occlusion-query destructor forwarding to common intrusive-list teardown.
// attributes: thunk
__int64 __fastcall orbis_occlusion_query_destruct(_QWORD *a1)
{
  return occlusion_query_destruct(a1);
}


/* 0x8E2900 */
// Deleting Orbis occlusion-query destructor.
double __fastcall orbis_occlusion_query_delete(_QWORD *a1)
{
  occlusion_query_destruct(a1);
  return sub_37BF50(a1);
}


/* 0x8E2920 */
// Allocates aligned query memory, begins the hardware query, and enables query collection.
__int64 __fastcall orbis_occlusion_query_begin(__int64 a1, __int64 a2)
{
  __int64 v4; // rax
  unsigned __int64 v5; // rdx
  unsigned __int64 *v6; // r15
  unsigned __int64 v7; // rdx

  v4 = 59528LL * *(_QWORD *)(a2 + 265616);
  v5 = *(_QWORD *)(a2 + v4 + 22320);
  v6 = (unsigned __int64 *)(a2 + v4 + 22320);
  if ( (unsigned int)((v5 - *(_QWORD *)(a2 + v4 + 22328)) >> 2) <= 0x44 )
  {
    if ( (*(unsigned __int8 (__fastcall **)(__int64, __int64, _QWORD))(a2 + v4 + 22336))(
           a2 + v4 + 22312,
           69,
           *(_QWORD *)(a2 + v4 + 22344)) == 0 )
    {
      v7 = 0;
      goto LABEL_6;
    }
    v5 = *v6;
  }
  v7 = (v5 - 256) & 0xFFFFFFFFFFFFFFF0LL;
  *v6 = v7;
LABEL_6:
  *(_QWORD *)(a1 + 64) = v7;
  sub_10CAB40(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312, 0);
  return sub_10CA130(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312, 1, 0);
}


/* 0x8E29E0 */
// Ends the hardware query and disables query collection.
__int64 __fastcall orbis_occlusion_query_end(__int64 a1, __int64 a2)
{
  sub_10CAB40(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312, 1);
  return sub_10CA130(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312, 0, 0);
}


/* 0x8E2A30 */
// Begins conditional rendering from this query's result address.
_DWORD *__fastcall orbis_occlusion_query_begin_conditional_render(__int64 a1, __int64 a2)
{
  return sub_10CAA50(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312, *(_QWORD *)(a1 + 64), 0, 1);
}


/* 0x8E2A60 */
// Ends conditional rendering on the active graphics command buffer.
__int64 __fastcall orbis_occlusion_query_end_conditional_render(__int64 a1, __int64 a2)
{
  return sub_10CAAE0(a2 + 59528LL * *(_QWORD *)(a2 + 265616) + 22312);
}

