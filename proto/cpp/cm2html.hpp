#ifndef CM2HTML_CM2HTML_HPP
#define CM2HTML_CM2HTML_HPP

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <cmark.h>
#include <srchilite/regexrulefactory.h>
#include <srchilite/langdefmanager.h>
#include <srchilite/formattermanager.h>

#include "pre-formatter.hpp"

namespace cm2html {
  using namespace std::string_literals;

  struct FileDeleter {
    void operator()(FILE * fp) { fclose(fp); }
  };
  struct CMarkNodeDeleter {
    void operator()(cmark_node * node) { cmark_node_free(node); }
  };
  struct CMarkIterDeleter {
    void operator()(cmark_iter * iter) { cmark_iter_free(iter); }
  };
  struct LegacyDeleter {
    void operator()(char * ptr) { std::free(ptr); }
  };

  class CMark2HTML {
    std::unique_ptr<cmark_node, CMarkNodeDeleter> doc_root;
    std::unique_ptr<char, LegacyDeleter> result_html;
    bool is_error;
    std::string err_msg;
    srchilite::RegexRuleFactory rule_factory;
    srchilite::LangDefManager lang_def_mgr;
    std::ostringstream fmt_strm;
    srchilite::FormatterManager fmt_mgr;

  public:
    CMark2HTML()
      : is_error{false}, lang_def_mgr{&rule_factory}
      , fmt_mgr{PreFormatter::shptr_t{new PreFormatter{fmt_strm}}}
    {
      PreFormatter::addAllFormatter(fmt_mgr, fmt_strm);
    }

  private:
    CMark2HTML(CMark2HTML const&) = delete;
    CMark2HTML & operator=(CMark2HTML const&) = delete;

  public:
    void process(std::string const& file_name);
    char const* getHTML() const { return result_html.get(); }
    bool isError() const { return is_error; }
    std::string const& whatError() const { return err_msg; }

  private:
    void highlightCodeBlock(cmark_node * node);
  };
}

#endif //CM2HTML_CM2HTML_HPP
