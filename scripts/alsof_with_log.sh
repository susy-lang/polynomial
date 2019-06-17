#!/usr/bin/env bash

ALSOF_PATH=$1
ALSOF_TMP_OUT=$2
shift
shift


$ALSOF_PATH $@ &> >(tail -n 100000 &> "$ALSOF_TMP_OUT") &

PID=$!

function cleanup()
{
	kill $PID
}

trap cleanup INT TERM

wait $PID

