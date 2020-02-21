%module pySwappedArgChecker

%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%{
#include "SwappedArgChecker.hpp"
%}

%include <windows.i>
%include "Compiler.hpp"
%include "SwappedArgChecker.hpp"

