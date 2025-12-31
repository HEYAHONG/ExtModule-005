#include "ld.common"
SECTIONS
{
  .HShellCommand :
  {
    . = ALIGN(4);
    mapping[flash_rodata]
    PROVIDE ( __hshell_command_start = ABSOLUTE(.) );
    KEEP (*(.HShellCommand))
    PROVIDE ( __hshell_command_end = ABSOLUTE(.) );
  } >default_rodata_seg

  .HRuntimeInit :
  {
    . = ALIGN(4);
    mapping[flash_rodata]
    PROVIDE ( __hruntime_init_start = ABSOLUTE(.) );
    KEEP (*(.HRuntimeInit))
    PROVIDE ( __hruntime_init_end = ABSOLUTE(.) );
  } >default_rodata_seg

  .HRuntimeLoop :
  {
    . = ALIGN(4);
    mapping[flash_rodata]
    PROVIDE ( __hruntime_loop_start = ABSOLUTE(.) );
    KEEP (*(.HRuntimeLoop))
    PROVIDE ( __hruntime_loop_end = ABSOLUTE(.) );
  } >default_rodata_seg

  .HRuntimeSymbol :
  {
    . = ALIGN(4);
	mapping[flash_rodata]
    PROVIDE ( __hruntime_symbol_start = ABSOLUTE(.) );
    KEEP (*(.HRuntimeSymbol))
    PROVIDE ( __hruntime_symbol_end = ABSOLUTE(.) );
  } >default_rodata_seg

 
}
