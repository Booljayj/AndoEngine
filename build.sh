#!/bin/sh
printf \\033c
make -C ./Library/
if [ $? -eq 0 ]
then
  make -C ./Tests/
fi
