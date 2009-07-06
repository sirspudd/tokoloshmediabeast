TEMPLATE = subdirs
SUBDIRS += tokoloshhead \
	   tokoloshtail
unix:system(mkdir -p $$PWD/bin)
win:system(md $$PWD/bin)
