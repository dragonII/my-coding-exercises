all: subdirs

SUBDIRS= string file log #config core db tcp_connection

subdirs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n || exit 1; done

clean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done

#install:
#	cp -auvf VERSION $(VERDIR)
#	for n in $(SUBDIRS); do $(MAKE) -C $$n install; done
