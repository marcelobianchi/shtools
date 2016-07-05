QFILE=BET1

all: 
	@echo "Running tests:"
	@make test
	@echo "Done"

test: ../../bin/sh2sac
	../../bin/sh2sac -h $(QFILE).QHD -b $(QFILE).QBN

clean: 
	rm -f *.sac