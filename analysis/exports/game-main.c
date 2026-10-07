// Top-level game routine: initialize once, then run frames until requested shutdown. Receives argc/argv/envp from start but does not use them in this build.
int __fastcall game_main(int argc, char **argv, char **envp)
{
  if ( game_initialize() )
  {
    while ( game_run_frame() )
      ;
  }
  return 0;
}
