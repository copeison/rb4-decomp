void __fastcall __noreturn start(unsigned int *initial_stack, void (*rtld_cleanup)())
{
  unsigned int v2; // r14d
  char **v4; // r15

  v2 = *initial_stack;
  v4 = (char **)(initial_stack + 2);
  init_env(initial_stack);
  atexit(rtld_cleanup);
  atexit(runtime_run_finalizers);
  runtime_run_initializers();
  LODWORD(rtld_cleanup) = game_main(v2, v4, nullptr);
  catchReturnFromMain((int)rtld_cleanup);
  exit((int)rtld_cleanup);
}
