#!/bin/sh

/usr/bin/jhcpeck
ck_ret=$?

if [ 0 != $ck_ret ]
then
  /usr/bin/cperegister
  exit 0
fi

