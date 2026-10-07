void __fastcall __noreturn start(unsigned int *a1, __int64 a2)
{
  unsigned int v2; // r14d
  unsigned int *v4; // r15
  __int64 v5; // rdi

  v2 = *a1;
  v4 = a1 + 2;
  sub_1243220(a1);
  sub_1243230(a2);
  sub_1243230(sub_12431A0);
  sub_20(v5);
  LODWORD(a2) = sub_3C0(v2, v4, 0);
  sub_1243240((unsigned int)a2);
  sub_1243250((unsigned int)a2);
  BUG();
}
