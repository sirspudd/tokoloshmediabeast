#!/bin/bash

dir="$1"
test -z "$dir" && dir="."

if xmllint -noout $dir/tokolosh.xml; then
    qdbusxml2cpp $dir/tokolosh.xml -a $dir/tokolosh_adaptor -c MediaAdaptor
    qdbusxml2cpp -c TokoloshInterface -p $dir/tokolosh_interface.h:$dir/tokolosh_interface.cpp $dir/tokolosh.xml
else
    xmllint $dir/tokolosh.xml
fi
