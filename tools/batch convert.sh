#!/bin/bash
for f in *.png; do python3 img2grb.py "$f"; done
