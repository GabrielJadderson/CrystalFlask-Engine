@echo off

set CommonCompilerFlags=-nologo -Gm- -GR- -EHa- -O2 -Oi /c

del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% build_number_incrementer.cpp 
link /nologo /INCREMENTAL:NO build_number_incrementer.obj
del build_number_incrementer.obj

