#!/bin/bash -x

if xmllint -noout $1/tokolosh.xml; then
    qdbusxml2cpp $1/tokolosh.xml -a $1/tokolosh_adaptor -c MediaAdaptor
    qdbusxml2cpp -c TokoloshInterface -p $1/tokolosh_interface.h:$1/tokolosh_interface.cpp $1/tokolosh.xml
else
    xmllint $1/tokolosh.xml
fi
