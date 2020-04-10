# Unicorn

Copypasted from [Unicorn Github page.](https://github.com/unicorn-bi/Unicorn-Suite-Hybrid-Black)

libunicorn.so was not compiled in manylinux and supports only Ubuntu, due to it we can not link BrainFlow against it and need to use dlopen instead.
Also it means that header must be splitted by two files manually - one file to contain data types and constants, another one to store function declarations(only for reference).
