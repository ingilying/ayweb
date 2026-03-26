#pragma once

#if !defined(AYWEB_EXPORT)

#if defined(AYWEB_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(AYWEB_COMPILE_LIBRARY)
#define AYWEB_EXPORT __declspec(dllexport)
#else
#define AYWEB_EXPORT __declspec(dllimport)
#endif

#else  // defined(_WIN32)
#if defined(AYWEB_COMPILE_LIBRARY)
#define AYWEB_EXPORT __attribute__((visibility("default")))
#else
#define AYWEB_EXPORT
#endif
#endif  // defined(_WIN32)

#else
#define AYWEB_EXPORT
#endif

#endif
