#!/bin/bash
ulimit -c unlimited
nohup ./dezhoukvredis -P 9999 1>std 2>err &
