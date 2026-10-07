__int64 __fastcall fmod_audio_input_manager_initialize_device_slots(__int64 a1, int a2)
{
  __int64 result; // rax
  int v4; // ebx
  __int64 v5; // r12

  result = sub_A1C40(&unk_19C8FC0);
  if ( a2 > 0 )
  {
    v4 = 0;
    do
    {
      v5 = sub_37BF40(16720);
      fmod_audio_input_device_construct(v5, v4);
      result = sub_A1F70(&unk_19C8FC0, v5);
      ++v4;
    }
    while ( a2 != v4 );
  }
  return result;
}
