#!/bin/sh
printf \\033c
gmake -C ./Library/ --no-print-directory
if [ $? -eq 0 ]
then
  gmake -C ./Tests/ --no-print-directory
fi
