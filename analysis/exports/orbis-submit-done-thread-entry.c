// Thread entry wrapper for the Orbis submit-done event loop.
__int64 __fastcall orbis_submit_done_thread_entry(__int64 a1)
{
  orbis_submit_done_thread_run(a1);
  return 0;
}
