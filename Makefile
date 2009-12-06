SUBDIRS = src c_src

all:
	for d in $(SUBDIRS); do					\
		(cd $$d && $(MAKE) $$xflag $@) || exit $$? ;	\
	done ;



