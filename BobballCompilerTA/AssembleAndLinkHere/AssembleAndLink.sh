#!/bin/bash
echo "Creating"$1".obj"
./nasm.exe -f win32 $1.asm
echo "Done"
echo ""
echo "Creating"$1".exe"
./link.exe /OUT:$1.exe msvcrtd.lib $1.obj
echo "Done"
echo ""
echo "Calling: ./"$1".exe"
echo ""
./$1.exe