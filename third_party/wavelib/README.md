# Wavelib

I copypasted this code from [this_link](https://github.com/rafat/wavelib). Its a great project but I had to make some changes here:

* it was a plain C code and to report errors it called exit(-1) instead returning exit code, for BrainFlow project its not acceptable, so I changed exit to exception and I had to change all this stuff to C++ intead plain C
* during changes above I've applied my own clang-format so now we can not even see a diff between two version
* add a few more checks
