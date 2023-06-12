#!/bin/bash

mkdir extracted 
rm -rf codes/*

find -name '*.zip' -exec bash -c '
    echo $name
' '{}' ';'

rm -rf extracted