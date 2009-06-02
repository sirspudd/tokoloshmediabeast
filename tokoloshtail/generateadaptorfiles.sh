#!/bin/bash 
if xmllint -noout ../shared/tokolosh.xml; then
    qdbusxml2cpp ../shared/tokolosh.xml -a tokolosh_adaptor -c MediaAdaptor
else
    xmllint ../shared/tokolosh.xml
fi
