#include "subhook.h"
#include "../winapi.h"
#include "subhook_private.h"

subhook_disasm_handler_t subhook_disasm_handler = NULL;

SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_src(subhook_t hook) {
  if (hook == NULL) {
    return NULL;
  }
  return hook->src;
}

SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_dst(subhook_t hook) {
  if (hook == NULL) {
    return NULL;
  }
  return hook->dst;
}

SUBHOOK_EXPORT void *SUBHOOK_API subhook_get_trampoline(subhook_t hook) {
  if (hook == NULL) {
    return NULL;
  }
  return hook->trampoline;
}

SUBHOOK_EXPORT int SUBHOOK_API subhook_is_installed(subhook_t hook) {
  if (hook == NULL) {
    return false;
  }
  return hook->installed;
}

SUBHOOK_EXPORT void SUBHOOK_API subhook_set_disasm_handler(
  subhook_disasm_handler_t handler) {
  subhook_disasm_handler = handler;
}

/*#ifndef SUBHOOK_SEPARATE_SOURCE_FILES

#if defined SUBHOOK_WINDOWS
  #include "subhook_windows.c"
#elif defined SUBHOOK_UNIX
  #include "subhook_unix.c"
#endif

#if defined SUBHOOK_X86 || defined SUBHOOK_X86_64
  #include "subhook_x86.c"
#endif

#endif*/
