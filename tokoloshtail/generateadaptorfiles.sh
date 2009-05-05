#!/bin/bash 
if xmllint -noout tokolosh.xml; then
    qdbusxml2cpp tokolosh.xml -a tokolosh_adaptor -c MediaAdaptor
else
    xmllint tokolosh.xml
fi
