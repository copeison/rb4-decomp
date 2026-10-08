__int64 __fastcall render_get_default_texture(__int64 a1, int a2, int a3)
{
  __int64 result; // rax
  __int64 *v4; // rax

  result = 0;
  switch ( a2 )
  {
    case 0:
      v4 = (__int64 *)(a1 + 8LL * a3 + 8);
      goto LABEL_9;
    case 1:
      v4 = (__int64 *)(a1 + 8LL * a3 + 64);
      goto LABEL_9;
    case 2:
      v4 = (__int64 *)(a1 + 8LL * a3 + 120);
      goto LABEL_9;
    case 3:
      v4 = (__int64 *)(a1 + 8LL * a3 + 176);
      goto LABEL_9;
    case 4:
      v4 = (__int64 *)(a1 + 8LL * a3 + 232);
      goto LABEL_9;
    case 5:
      v4 = (__int64 *)(a1 + 8LL * a3 + 288);
      goto LABEL_9;
    case 7:
      v4 = (__int64 *)(a1 + 8LL * a3 + 344);
LABEL_9:
      result = *v4;
      break;
    default:
      return result;
  }
  return result;
}
