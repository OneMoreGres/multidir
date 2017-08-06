#!/bin/bash

if [ $# == 0 ]; then echo -e "Usage: $0 <changelog>.\nChangelog must separate version entries with '---'"; exit 0; fi

while read -r line; do
    if [ "${line:0:3}" == "---" ]; then exit 0; fi
    echo "$line"
done < "$1"
