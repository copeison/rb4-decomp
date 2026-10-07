void __fastcall fmod_audio_stream_generator_request_pause(__int64 a1)
{
  *(_BYTE *)(a1 + 256) = 1;
}
