#!/bin/bash

#inspired by http://scott.sherrillmix.com/blog/programmer/converting-eps-to-png-easily/


    for f in `ls DATA/*.eps`; do
         convert -density 100 $f -flatten ${f%.*}.png;
    done 


