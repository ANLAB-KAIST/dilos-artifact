#!/bin/bash
set -e
memaslap -s $$sut.ip:11211 --threads=$$tester.memaslap.threads --concurrency=$$tester.memaslap.concurrency --time=$$tester.memaslap.duration
