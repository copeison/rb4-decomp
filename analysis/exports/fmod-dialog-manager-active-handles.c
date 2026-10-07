void __fastcall fmod_dialog_manager_active_handles(__int64 a1, _QWORD *a2)
{
  int **v2; // rcx
  int *v3; // rdx
  __int64 v4; // r15
  __int64 v5; // r12
  int *v6; // rbx
  int v7; // r13d
  __int64 v8; // r14
  int **v9; // rbx
  __int64 v10; // rax
  int *v11; // rsi
  __int64 v12; // rdi
  char *v13; // rbx
  int *v14; // [rsp+0h] [rbp-50h]
  __int64 v15; // [rsp+8h] [rbp-48h]
  __int64 v16; // [rsp+10h] [rbp-40h]
  _QWORD *v18; // [rsp+20h] [rbp-30h]

  v2 = (int **)a2;
  v3 = (int *)*a2;
  a2[1] = *a2;
  if ( *(int *)(a1 + 8) > 0 )
  {
    v4 = 0;
    v5 = 36;
    v6 = v3;
    v15 = a1;
    v18 = a2 + 3;
    do
    {
      v7 = *(_DWORD *)(*(_QWORD *)(a1 + 64) + v5);
      if ( v7 < 0 )
      {
        if ( v6 >= v2[2] )
        {
          v8 = ((char *)v6 - (char *)v3) >> 1;
          if ( v6 == v3 )
            v8 = 1;
          if ( v8 != 0 )
          {
            v9 = v2;
            v10 = sub_252CF0(v18, 4 * v8, 0);
            v11 = *v9;
            v6 = v9[1];
            v12 = v10;
            v16 = v10;
          }
          else
          {
            v11 = v3;
            v12 = 0;
            v16 = 0;
          }
          v13 = (char *)((char *)v6 - (char *)v11);
          v14 = (int *)v12;
          memmove(v12, v11, v13);
          *(_DWORD *)&v13[v16] = v7;
          v6 = (int *)&v13[v16 + 4];
          v2 = (int **)a2;
          if ( *a2 != 0 )
          {
            sub_252D30(v18, *a2, a2[2] - *a2);
            v2 = (int **)a2;
          }
          v3 = v14;
          *v2 = v14;
          v2[1] = v6;
          v2[2] = (int *)(v16 + 4 * v8);
          a1 = v15;
        }
        else
        {
          v2[1] = v6 + 1;
          *v6++ = v7;
        }
      }
      ++v4;
      v5 += 416;
    }
    while ( v4 < *(int *)(a1 + 8) );
  }
}
