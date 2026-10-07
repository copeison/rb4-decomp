// Lazily interns the 22 StagePresenceId names and returns the Symbol value for the requested ID.
__int64 stage_presence_id_to_symbol(int id)
{
  if ( byte_1AE5930 == 0 && (unsigned int)_cxa_guard_acquire(&byte_1AE5930) != 0 )
  {
    sub_256FD0(qword_1AE5880, "kBandOverdrive");
    sub_256FD0(&unk_1AE5888, "kGreatGuitarSolo");
    sub_256FD0(&unk_1AE5890, "kMaxStreakTrack");
    sub_256FD0(&unk_1AE5898, "kMaxStreakBand");
    sub_256FD0(&unk_1AE58A0, "kBandUnity");
    sub_256FD0(&unk_1AE58A8, "kUpstrummer");
    sub_256FD0(&unk_1AE58B0, "kHopoMaster");
    sub_256FD0(&unk_1AE58B8, "kFullOfFills");
    sub_256FD0(&unk_1AE58C0, "kSuperSavior");
    sub_256FD0(&unk_1AE58C8, "kVocalFreestyler");
    sub_256FD0(&unk_1AE58D0, "kHarmonizer");
    sub_256FD0(&unk_1AE58D8, "kVocalCrowdWork");
    sub_256FD0(&unk_1AE58E0, "kImprovGuitarSixteenths");
    sub_256FD0(&unk_1AE58E8, "kImprovGuitarEights");
    sub_256FD0(&unk_1AE58F0, "kImprovGuitarLicks");
    sub_256FD0(&unk_1AE58F8, "kImprovGuitarHeldNotes");
    sub_256FD0(&unk_1AE5900, "kImprovGuitarTapping");
    sub_256FD0(&unk_1AE5908, "kTakingRequests");
    sub_256FD0(&unk_1AE5910, "kGreatBassSolo");
    sub_256FD0(&unk_1AE5918, "kGreatDrumSolo");
    sub_256FD0(&unk_1AE5920, "kSoloFiveStars");
    sub_256FD0(&unk_1AE5928, "kImprovGuitarGeneral");
    _cxa_guard_release(&byte_1AE5930);
  }
  return qword_1AE5880[id];
}
