TEMPLATE = subdirs
SUBDIRS += head \
	   tail
unix:system(mkdir -p $$PWD/bin)
win:system(md $$PWD/bin)
