#include <nuttx/config.h>
SECTIONS
{
  .HShellCommand :
  {
    . = ALIGN(4);
    PROVIDE ( __hshell_command_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HShellCommand)))
    PROVIDE ( __hshell_command_end = ABSOLUTE(.) );
  } >default_rodata_seg

  .HRuntimeInit :
  {
    . = ALIGN(4);
    PROVIDE ( __hruntime_init_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HRuntimeInit)))
    PROVIDE ( __hruntime_init_end = ABSOLUTE(.) );
  } >default_rodata_seg

  .HRuntimeLoop :
  {
    . = ALIGN(4);
    PROVIDE ( __hruntime_loop_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HRuntimeLoop)))
    PROVIDE ( __hruntime_loop_end = ABSOLUTE(.) );
  } >default_rodata_seg

  .HRuntimeSymbol :
  {
    . = ALIGN(4);
    PROVIDE ( __hruntime_symbol_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HRuntimeSymbol)))
    PROVIDE ( __hruntime_symbol_end = ABSOLUTE(.) );
  } >default_rodata_seg

 
}
