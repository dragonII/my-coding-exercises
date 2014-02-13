#!/bin/sh

gcc -Wall $@ $(pkg-config --cflags --libs gstreamer-1.0)
