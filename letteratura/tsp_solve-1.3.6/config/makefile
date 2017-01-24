#----------------------------------------------------------------------------#
#                       Tsp_Solve Configure makefile                         #
#----------------------------------------------------------------------------#
CONFIG = ../config

all: conf.h.in conf.in conf

conf.in:
	- rm -f conf.in
	ln -s $(CONFIG)ure.in conf.in

conf.h.in: conf.in
	autoheader conf.in >conf.h.in

conf: conf.in conf.h.in
	autoconf conf.in >conf
	chmod u+x conf

diff: all
	- diff conf.h.in $(CONFIG).h.in
	- diff conf $(CONFIG)ure

clean:
	- rm -f conf.in conf.h.in conf config.cache config.log config.status
	- rm -f Makefile config.h

install: all
	mv conf.h.in $(CONFIG).h.in
	mv conf $(CONFIG)ure
