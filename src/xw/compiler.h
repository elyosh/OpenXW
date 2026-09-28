#ifndef XW_COMPILER_H
#define XW_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Original x86 calling conventions and compiler attributes. */
#if defined(_MSC_VER)
#define XW_STDCALL __stdcall
#if _MSC_VER >= 1200
#define XW_NORETURN __declspec(noreturn)
#else
#define XW_NORETURN
#endif
#elif defined(__i386__)
#define XW_STDCALL __attribute__((stdcall))
#define XW_NORETURN __attribute__((noreturn))
#else
#define XW_STDCALL
#define XW_NORETURN __attribute__((noreturn))
#endif

#ifdef __cplusplus
}
#endif

#endif
