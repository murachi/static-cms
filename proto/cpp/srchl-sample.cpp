#include <string>
#include <sstream>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <srchilite/formatter.h>
#include <srchilite/langdefmanager.h>
#include <srchilite/regexrulefactory.h>
#include <srchilite/sourcehighlighter.h>
#include <srchilite/formattermanager.h>
#include <srchilite/formatterparams.h>

using namespace std::string_literals;

std::string const sampleSourceCode = R"(#!/usr/bin/perl
use strict;
use warnings;

# 関数
sub hoge {
  my $hoge = shift;
  $hoge =~ s/piyo/hoge fuga PIYO!!/g;
  my $result = "[hoge] $hoge";
  print "$result\n";
  return $result;
}

# 関数呼び出し
hoge("piyo piyo");

__EOF__
=pod
=head1 hoge
=head2 これは

なんだろうね…。

=cut

)"s;

std::string const specialElems[] =
  { "keyword"s, "string"s, "type"s, "comment"s, "symbol"s, "number"s, "preproc"s, "regexp"s };

class ModernHTMLFormatter: public srchilite::Formatter {
  std::ostream & ostrm;
  std::string elem;

public:
  explicit ModernHTMLFormatter(std::ostream & ostrm_, std::string const& elem_ = "normal"s)
    : ostrm{ostrm_}, elem{elem_}
  {}
  explicit ModernHTMLFormatter(std::ostream & ostrm_, std::string && elem_)
    : ostrm{ostrm_}, elem{elem_}
  {}

  ModernHTMLFormatter(ModernHTMLFormatter const&) = delete;
  ModernHTMLFormatter & operator=(ModernHTMLFormatter const&) = delete;

  virtual void format(std::string const& s, srchilite::FormatterParams const* params = 0)
  {
    if (elem != "normal"s) {
      ostrm << R"(<span class=")"s << elem << R"(">)"s << s << "</span>"s;
    }
    else {
      ostrm << s;
    }
  }
};

using ModernHTMLFormatterPtr = boost::shared_ptr<ModernHTMLFormatter>;

int main(int, char const**)
{
  srchilite::RegexRuleFactory rule_factory;
  srchilite::LangDefManager lang_def_mgr{&rule_factory};

  srchilite::SourceHighlighter highlighter{lang_def_mgr.getHighlightState("perl.lang")};

  std::ostringstream ostrm;
  srchilite::FormatterManager formatter_mgr{ModernHTMLFormatterPtr{new ModernHTMLFormatter{ostrm}}};
  for (auto const& elem: specialElems) {
    formatter_mgr.addFormatter(elem, ModernHTMLFormatterPtr{new ModernHTMLFormatter{ostrm, elem}});
  }
  highlighter.setFormatterManager(&formatter_mgr);

  srchilite::FormatterParams params;
  highlighter.setFormatterParams(&params);

  std::istringstream istrm{sampleSourceCode};
  std::string line;
  while (istrm.good()) {
    std::getline(istrm, line);
    params.start = 0;
    highlighter.highlightParagraph(line);
    ostrm << std::endl;
  }

  std::cout << ostrm.str() << std::endl;

  return 0;
}
