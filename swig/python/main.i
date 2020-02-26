%module pySwappedArgChecker

%begin %{
#ifdef _MSC_VER
#define SWIG_PYTHON_INTERPRETER_NO_DEBUG
#endif
%}

%include "swapped_arg_checker.i"
%include "identifier_splitting.i"

