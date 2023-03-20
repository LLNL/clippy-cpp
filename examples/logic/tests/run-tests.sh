#!/usr/bin/env bash

for tst in *.json; do
  echo "testing ../../../build/examples/logic/testeval <$tst"
  ../../../build/examples/logic/testeval <$tst
  res=$?
  if [[ $res -ne 0 ]] ; then
    echo "$res"
    exit 1
  fi
done

echo "qed."

