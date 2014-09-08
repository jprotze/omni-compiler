#!/usr/bin/perl
while(<>) {
    s/(\W|^)__const(\W?|$)/\1\2/g;
    s/(\W|^)__restrict(\W?|$)/\1\2/g;
    s/(\W|^)__wur(\W?|$)/\1\2/g;
    s/(\W|^)__attribute__\s*\([^(]*\([^(]*\([^)]+\)[^)]*\)[^)]*\)(\W?|$)/\1\2/g;
    s/(\W|^)__attribute__\s*\([^(]*\([^)]+\)[^)]*\)(\W?|$)/\1\2/g;
    print;
}

