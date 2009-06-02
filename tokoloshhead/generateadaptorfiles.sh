#!/bin/bash -x
qdbusxml2cpp -c TokoloshInterface -p tokolosh_interface.h:tokolosh_interface.cpp ../shared/tokolosh.xml
