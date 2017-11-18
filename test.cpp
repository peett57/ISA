#!/bin/bash
        for i in `seq 1 10`;
        do
                ./isamon -n 10.190.22.160/22 -w 50 -t -p $i
        done  