.phony all:
all: compile

compile:
	gcc -Wall helper.c diskinfo.c -o diskinfo
	gcc -Wall helper.c disklist.c -o disklist
	gcc -Wall helper.c diskget.c -o diskget

.phony clean:
clean:
	rm diskinfo disklist diskget

.PHONY: run1
run1:
	./diskinfo disks/disk.IMA
	./disklist disks/disk.IMA
	./diskget disks/disk.IMA ans1.pdf

.PHONY: run2
run2:
	./diskinfo disks/Image2020.IMA
	./disklist disks/Image2020.IMA
	./diskget disks/Image2020.IMA ans1.pdf