#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>

#include "cm2html.hpp"

using namespace std::string_literals;

namespace cm2html {
  void CMark2HTML::process(std::string const& file_name)
  {
    std::unique_ptr<FILE, FileDeleter> fin{std::fopen(file_name.c_str(), "rb")};
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
  {
    std::string language = cmark_node_get_fence_info(node);
    std::string lang_file = lang_map.getMappedFileName(language);
    if (lang_file.empty()) {
      // 言語指定がないか、または未対応の言語指定の場合は、何もしない。
      return;
    }

    std::istringstream istrm{cmark_node_get_literal(node)};
    std::ostringstream ostrm;

    src_highlight.highlight(istrm, ostrm, lang_file);

    auto * new_node = cmark_node_new(CMARK_NODE_HTML_BLOCK);
    cmark_node_set_literal(new_node, ostrm.str().c_str());
    cmark_node_replace(node, new_node);
    cmark_node_free(node);
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
