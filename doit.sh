#!/bin/bash

if [ "$1" = "help" ]
then
   echo "doit.sh [clean]"
elif [ "$1" = "clean" ]
then
   rm -rf first.txt second.txt
   echo "Cleaned"
else
   echo "Running both executables"
   echo "First output will be first.txt"
   echo "Second output will be second.txt"
   ./a.out 127.0.0.1 127.0.0.1 7086 7087 500 > first.txt &
   ./a.out 127.0.0.1 127.0.0.1 7087 7086 250 > second.txt &

   wait
   echo "Both executables completed"
fi
