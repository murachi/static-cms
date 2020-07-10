build: cmark-sample

cmark-sample: cmark-sample.cpp
	g++ -o $@ $< -l cmark

run: cmark-sample
	./$<

clean:
	rm -f ./cmark-sample

.PHONY: build run clean
