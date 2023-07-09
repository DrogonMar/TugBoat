#!/bin/bash
set -e
borz c -p Windows
winedbg --gdb --port 3989 --no-start TestProj-Windows-debug.exe lol eg
