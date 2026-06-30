    PROVIDE ( __hshell_command_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HShellCommand)))
    PROVIDE ( __hshell_command_end = ABSOLUTE(.) );
    . = ALIGN(4);
    PROVIDE ( __hruntime_init_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HRuntimeInit)))
    PROVIDE ( __hruntime_init_end = ABSOLUTE(.) );
    . = ALIGN(4);
    PROVIDE ( __hruntime_loop_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HRuntimeLoop)))
    PROVIDE ( __hruntime_loop_end = ABSOLUTE(.) );
    . = ALIGN(4);
    PROVIDE ( __hruntime_symbol_start = ABSOLUTE(.) );
    KEEP ( *(SORT (.HRuntimeSymbol)))
    PROVIDE ( __hruntime_symbol_end = ABSOLUTE(.) );
