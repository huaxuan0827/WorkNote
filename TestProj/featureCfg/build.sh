#!/bin/bash

# build 
gcc -g -o cJSON.o -c cJSON.c -I./
gcc -g -o scandev.o -c scandev.c -I./
gcc -g -o test_feature.o -c test_feature.c -I./

gcc -g -o test_feature test_feature.o scandev.o cJSON.o -lpthread -lm

