__int64 __fastcall thread_affinity_find_group(__int64 a1, _QWORD *a2)
{
  __int64 v3; // r13
  int v4; // ecx
  int v5; // ebx
  _QWORD *v6; // r15
  unsigned __int64 v7; // rax
  __int64 v8; // rdi
  int v9; // r12d
  int v10; // eax
  char *v11; // rax
  _QWORD *v13; // [rsp+8h] [rbp-48h]
  unsigned __int64 v14; // [rsp+10h] [rbp-40h]
  int v16; // [rsp+24h] [rbp-2Ch]

  v3 = 0;
  v4 = -1;
  v5 = 0;
  v6 = &unk_19B0628;
  v14 = unk_19E8818;
  LODWORD(v7) = unk_19B0410;
  do
  {
    if ( (_DWORD)v7 == 0 )
      break;
    v13 = v6;
    v8 = *((_QWORD *)&unk_19B0410 + 1057 * v3 + 1);
    if ( v8 != 0 )
    {
      v9 = 0;
      do
      {
        v16 = v4;
        v10 = strcmp(v8, a1);
        v4 = v16;
        v8 = *v6;
        if ( v10 == 0 )
        {
          v5 = v3;
          v4 = v9;
        }
        ++v9;
        v6 += 66;
      }
      while ( v8 != 0 );
    }
    ++v3;
    v7 = *((unsigned int *)&unk_19B0410 + 2114 * v3);
    v6 = v13 + 1057;
  }
  while ( v14 >= v7 );
  if ( v4 == -1 )
    v11 = (char *)&unk_19B4620;
  else
    v11 = (char *)&unk_19B0410 + 8456 * v5 + 528 * v4 + 8;
  *a2 = v11;
  return *((_QWORD *)v11 + 1);
}
