# Audio Spectrum Analyzer


## Cross compiling CppUTest
For now CppUTest will be included as a precompiled binary. In the future, it would be nice to include it as part of the Yocto build.

Below are the steps to compile CppUTest
```
git clone https://github.com/cpputest/cpputest.git
cd cpputest
mkdir cpputest_build
cd cpputest_build
autoreconf .. -i
../configure --host=arm
make
```

These steps are almost identical to that currently describes by the readme in the CppUTest repository. The only difference is the --host=arm flag used to indicate that we are cross compiling
