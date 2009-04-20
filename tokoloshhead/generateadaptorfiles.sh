#!/bin/bash -x
qdbusxml2cpp -c TokoloshInterface -p tokolosh_interface.h:tokolosh_interface.cpp ../tokoloshtail/tokolosh.xml
