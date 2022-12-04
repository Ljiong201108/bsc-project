#!/bin/bash

make clean
make host
make kernel 
make link 
# make package
make run