#!/bin/bash
ulimit -c unlimited
./dezhoukvredis -P 9999 1>std 2>err &
