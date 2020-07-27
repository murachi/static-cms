CM2HTML_CPP = cm2html.cpp
CM2HTML_HPP = cm2html.hpp

ENV_DATADIR = "/usr/share/source-highlight"s

build: cmark-sample srchl-sample cm2html

cmark-sample: cmark-sample.cpp
	g++ -o $@ $^ -l cmark

srchl-sample: srchl-sample.cpp
	g++ -o $@ $^ -l source-highlight

cm2html: $(CM2HTML_CPP) $(CM2HTML_HPP)
	g++ -o $@ -D 'DATADIR=$(ENV_DATADIR)' $(CM2HTML_CPP) -l cmark -l source-highlight

clean:
	rm -f *.o *.a *.so cmark-sample srchl-sample cm2html

.PHONY: build clean
