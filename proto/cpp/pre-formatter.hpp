#ifndef CM2HTML_PRE_FORMATTER_HPP
#define CM2HTML_PRE_FORMATTER_HPP

#include <string>
#include <memory>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <srchilite/formatter.h>
#include <srchilite/formattermanager.h>

namespace cm2html {
  using namespace std::string_literals;

  /// <pre> タグの中身を生成するフォーマッタ
  class PreFormatter : public srchilite::Formatter {
    std::ostream & out_target;
    std::string element_kind;

    static const std::string supportedElements[];

  public:
    class EscapeHTML {
      std::string text;
      static const std::unordered_map<char, std::string> entities;
    public:
      EscapeHTML() = delete;
      explicit EscapeHTML(std::string const& html) : text{html} {}
      std::ostream & operator()(std::ostream & ostrm) const
      {
        int pre = 0;
        for (;;) {
          int cur = text.find_first_of(R"(<>&")"s, pre);
          ostrm << text.substr(pre, cur - pre);
          if (cur == std::string::npos) break;
          ostrm << entities.at(text[cur]);
          pre = cur + 1;
        }
        return ostrm;
      }
    };

    explicit PreFormatter(std::ostream & ostrm, std::string const& elem = "normal"s)
      : out_target{ostrm}, element_kind{elem}
    {}
    explicit PreFormatter(std::ostream & ostrm, std::string && elem)
      : out_target{ostrm}, element_kind{elem}
    {}

    PreFormatter(PreFormatter const&) = delete;
    PreFormatter & operator=(PreFormatter const&) = delete;

    virtual void format(std::string const& text, srchilite::FormatterParams const* params = 0);

    using shptr_t = boost::shared_ptr<PreFormatter>;

    static srchilite::FormatterManager & addAllFormatter(
      srchilite::FormatterManager & fmt_mgr, std::ostringstream & ostrm);
  };

  inline std::ostream & operator<<(std::ostream & ostrm, PreFormatter::EscapeHTML const& manip) { return manip(ostrm); }

}

#endif  //CM2HTML_PRE_FORMATTER_HPP
