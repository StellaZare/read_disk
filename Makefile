.phony all:
all: compile

compile:
	gcc -Wall helper.c diskinfo.c -o diskinfo
	gcc -Wall helper.c disklist.c -o disklist
	gcc -Wall helper.c diskget.c -o diskget

.phony clean:
clean:
	rm diskinfo disklist diskget