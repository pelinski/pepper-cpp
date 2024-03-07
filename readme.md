# Pepper getting-started cpp version

This is the cpp version of the Pepper `getting-started` example, which was originally in Pd. The Debouncer class I took from the Bela C++ Real-Time Audio Programming course ([lecture videos](https://www.youtube.com/watch?v=aVLRUyPBBJk&list=PLCrgFeG6pwQmdbB6l3ehC8oBBZbatVoz3), [repo](https://github.com/BelaPlatform/bela-online-course/tree/master/code-examples/adsr-class)).

## Usage

With the Bela connected to your laptop, run the following commands in the terminal:

```bash
git clone https://github.com/pelinski/pepper-cpp
cd pepper-cpp && sh copy.sh
```

this will copy the project to your Bela. You can run it from the IDE or from the terminal with:

```bash
ssh root@bela.local "make -C Bela stop Bela PROJECT=pepper-cpp run"
```
