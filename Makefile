.PHONY: all examples test clean

all: examples

examples:
	./bin/holyc examples/Hello.HC
	./bin/holyc examples/Graphics.HC

test:
	python3 -m unittest discover -s tests -v
	./bin/holyc examples/Hello.HC
	./bin/holyc examples/Graphics.HC
	./.holyc-build/Hello
	SDL_VIDEODRIVER=dummy ./.holyc-build/Graphics

clean:
	rm -rf .holyc-build
