build: cmark-sample srchl-sample

cmark-sample: cmark-sample.cpp
	g++ -o $@ $< -l cmark

srchl-sample: srchl-sample.cpp
	g++ -o $@ $< -l source-highlight

clean:
	rm -f *.o *.a *.so cmark-sample srchl-sample

.PHONY: build clean
