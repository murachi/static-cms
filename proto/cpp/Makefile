CM2HTML_CPP = cm2html.cpp pre-formatter.cpp
CM2HTML_HPP = cm2html.hpp pre-formatter.hpp

build: cmark-sample srchl-sample cm2html

cmark-sample: cmark-sample.cpp
	g++ -o $@ $^ -l cmark

srchl-sample: srchl-sample.cpp
	g++ -o $@ $^ -l source-highlight

cm2html: $(CM2HTML_CPP) $(CM2HTML_HPP)
	g++ -o $@ $(CM2HTML_CPP) -l cmark -l source-highlight

clean:
	rm -f *.o *.a *.so cmark-sample srchl-sample cm2html

.PHONY: build clean
