bool __fastcall special_pad_reader_take_calibration_samples(void *self, void *output)
{
  _DWORD *v4; // rbx

  v4 = (_DWORD *)sub_8CF3A0(self);
  scePthreadMutexLock(v4 + 2);
  ++*v4;
  memcpy(output, (char *)self + 8208, 260);
  *((_DWORD *)self + 2116) = 0;
  --*v4;
  scePthreadMutexUnlock(v4 + 2);
  return true;
}
