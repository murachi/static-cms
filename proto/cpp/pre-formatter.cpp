#include <unordered_map>
#include <string>

#include "pre-formatter.hpp"

namespace cm2html {
  const std::string PreFormatter::supportedElements[] = {
    "context"s, "keyword"s, "type"s, "usertype"s, "string"s, "regexp"s, "specialchar"s,
    "comment"s, "number"s, "preproc"s, "symbol"s, "function"s, "cbracket"s, "todo"s, "code"s,
    "predef_var"s, "predef_func"s, "classname"s, "linenum"s, "url"s, "date"s, "time"s, "ip"s,
    "variable"s, "italics"s, "bold"s, "underline"s, "fixed"s, "argument"s, "optionalargument"s,
    "math"s, "bibtex"s, "oldfile"s, "newfile"s, "difflines"s, "selector"s, "property"s,
    "value"s, "atom"s, "meta"s, "path"s, "label"s, "error"s, "warning"s, "cuketag"s,
    "gherken"s, "given"s, "when"s, "then"s, "and_but"s, "table"s,
  };

  const std::unordered_map<char, std::string> PreFormatter::EscapeHTML::entities = {
    { '<', "&lt;"s },
    { '>', "&gt;"s },
    { '&', "&amp;"s },
    { '"', "&quot;"s },
  };

  void PreFormatter::format(std::string const& text, srchilite::FormatterParams const* params)
  {
    if (element_kind == "normal"s) out_target << EscapeHTML{text};
    else if (element_kind == "url"s) {
      out_target << R"(<a href=")"s << EscapeHTML{text} << R"(">)"s << EscapeHTML{text} << "</a>"s;
    }
    else {
      out_target << R"(<span class=")"s << element_kind << R"(">)"s << EscapeHTML{text} << "</span>"s;
    }
  }

  srchilite::FormatterManager & PreFormatter::addAllFormatter(srchilite::FormatterManager & fmt_mgr, std::ostringstream & ostrm)
  {
    for (auto const& elem: supportedElements) {
      fmt_mgr.addFormatter(elem, shptr_t{new PreFormatter{ostrm, elem}});
    }
    return fmt_mgr;
  }
}
