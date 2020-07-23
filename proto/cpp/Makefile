build: cmark-sample srchl-sample cm2html

cmark-sample: cmark-sample.cpp
	g++ -o $@ $^ -l cmark

srchl-sample: srchl-sample.cpp
	g++ -o $@ $^ -l source-highlight

cm2html: cm2html.cpp pre-formatter.cpp
	g++ -o $@ $^ -l cmark -l source-highlight

clean:
	rm -f *.o *.a *.so cmark-sample srchl-sample cm2html

.PHONY: build clean
