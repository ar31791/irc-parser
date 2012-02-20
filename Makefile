default:
	$(MAKE) -C src

test:
	$(MAKE) -C test run

clean:
	$(MAKE) -C src clean