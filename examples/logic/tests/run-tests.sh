#!/usr/bin/env bash

for tst in *.json; do
  echo "testing ../../../b2/examples/logic/testeval <$tst"
  ../../../b2/examples/logic/testeval <$tst
  res=$?
  if [[ $res -ne 0 ]] ; then
    echo "$res"
    exit 1
  fi
done

echo "qed."

