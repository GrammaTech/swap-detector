#ifndef GT_COMPILER_H
#define GT_COMPILER_H

#ifndef SWAPPED_ARG_EXPORT
#if defined(_MSC_VER)
#define SWAPPED_ARG_EXPORT __declspec(dllexport)
#else
#define SWAPPED_ARG_EXPORT
#endif // _MSC_VER
#endif // SWAPPED_ARG_EXPORT

#endif // GT_COMPILER_H
