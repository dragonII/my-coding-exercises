#!/bin/sh

gcc $@ $(pkg-config --cflags --libs gstreamer-1.0)
