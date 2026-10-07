// Marks every unhandled command-line entry beginning with a dash as handled.
void command_line_mark_switches_handled(void *arguments)
{
  __int64 v1; // rax
  __int64 v2; // rcx

  v1 = *(_QWORD *)arguments;
  v2 = *((_QWORD *)arguments + 1);
  if ( *(_QWORD *)arguments != v2 )
  {
    do
    {
      if ( **(_BYTE **)v1 == 45 && *(_BYTE *)(v1 + 8) == 0 )
        *(_BYTE *)(v1 + 8) = 1;
      v1 += 16;
    }
    while ( v2 != v1 );
  }
}
