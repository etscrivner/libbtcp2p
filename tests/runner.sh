#!/usr/bin/env bash
set -e
printf "  %s\n" "$1"

$1 #> /dev/null
