// Bound buffered-output callback: read one FMOD mixer block into the recording target's float buffer.
__int64 __fastcall fmod_recording_output_callback_invoke(__int64 a1, _QWORD *a2)
{
  return (*(__int64 (__fastcall **)(_QWORD, _QWORD, _QWORD))(*a2 + 8LL))(
           *a2,
           *(_QWORD *)(*(_QWORD *)(a1 + 8) + 280LL),
           *(unsigned int *)(*(_QWORD *)(a1 + 8) + 204LL));
}
