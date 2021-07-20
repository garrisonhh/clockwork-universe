#!/usr/bin/env bash
cppcheck --std=c99 --enable=all --inline-suppr --suppress=unusedFunction --suppress=variableScope -isystem./external -isystem./external/*/include/ src
