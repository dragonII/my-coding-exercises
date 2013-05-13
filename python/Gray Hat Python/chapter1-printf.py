from ctypes import *

libc = CDLL("libc.so.6")
message_string = "Hello World!\n"
libc.printf("Testing: %s", message_string)
