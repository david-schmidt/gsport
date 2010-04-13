#!/bin/sh
cd target/site
perl -i.orig -p ../../removeMaven.re developing.html
perl -i.orig -p ../../removeMaven.re index.html
perl -i.orig -p ../../removeMaven.re ethernet.html
cd ../..
