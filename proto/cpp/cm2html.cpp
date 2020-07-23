#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <srchilite/sourcehighlighter.h>
#include <srchilite/parserexception.h>
#include <srchilite/formatterparams.h>

#include "pre-formatter.hpp"
#include "cm2html.hpp"

using namespace std::string_literals;

namespace cm2html {
  void CMark2HTML::process(std::string const& file_name)
  {
    std::unique_ptr<FILE, FileDeleter> fin{fopen(file_name.c_str(), "rb")};
    if (!fin) {
      doc_root.reset();
      result_html.reset();
      is_error = true;
      err_msg = "ファイル "s + file_name + " を開けません。"s;
      return;
    }

    doc_root.reset(cmark_parse_file(fin.get(), CMARK_OPT_VALIDATE_UTF8));

    cmark_event_type ev_type;
    std::unique_ptr<cmark_iter, CMarkIterDeleter> iter{cmark_iter_new(doc_root.get())};
    while ((ev_type = cmark_iter_next(iter.get())) != CMARK_EVENT_DONE) {
      auto * cur = cmark_iter_get_node(iter.get());
      if (cmark_node_get_type(cur) == CMARK_NODE_CODE_BLOCK) {
        highlightCodeBlock(cur);
      }
    }

    result_html.reset(cmark_render_html(doc_root.get(), CMARK_OPT_UNSAFE));
  }

  void CMark2HTML::highlightCodeBlock(cmark_node * node)
  try {
    std::string language = cmark_node_get_fence_info(node);
    srchilite::SourceHighlighter highlighter{lang_def_mgr.getHighlightState((language + ".lang"s).c_str())};
    highlighter.setFormatterManager(&fmt_mgr);
    srchilite::FormatterParams prms;
    highlighter.setFormatterParams(&prms);

    fmt_strm.str(R"(<pre><code class="lang )"s + language + R"(">)");
    fmt_strm.seekp(0, std::ios_base::end);

    std::istringstream istrm{cmark_node_get_literal(node)};
    std::string line;
    while (istrm.good()) {
      std::getline(istrm, line);
      prms.start = 0;
      highlighter.highlightParagraph(line);
      fmt_strm << std::endl;
    }
    fmt_strm.seekp(-1, std::ios_base::end);  // 最後に空行が入っちゃうのを除去
    fmt_strm << "</code></pre>";

    auto * new_node = cmark_node_new(CMARK_NODE_HTML_BLOCK);
    cmark_node_set_literal(new_node, fmt_strm.str().c_str());
    cmark_node_replace(node, new_node);
    cmark_node_free(node);
  }
  catch (srchilite::ParserException) {
    // パーサーエラーの場合は何もしない (単に言語指定がない場合も含む)
  }
}

std::string getUsageText(std::string const& cmd_name)
{
  return "Usage:\n"s
    + "    "s + cmd_name + " in_file.md [out_file.html]\n"s;
}

int main(int argc, char const* const argv[])
{
  if (argc < 2) {
    std::cerr << argv[0] << ": Markdown ファイルを指定してください。" << std::endl;
    std::cerr << getUsageText(argv[0]) << std::endl;
    return 1;
  }

  cm2html::CMark2HTML conv;
  std::string in_file{argv[1]};
  conv.process(in_file);
  if (conv.isError()) {
    std::cerr << argv[0] << ": "s << conv.whatError() << std::endl;
    std::cerr << getUsageText(argv[0]) << std::endl;
    return 1;
  }

  std::string out_file{argc >= 3 ? std::string{argv[2]} : in_file + ".html"s};
  std::ofstream fout{out_file};
  fout << conv.getHTML();

  return 0;
}
