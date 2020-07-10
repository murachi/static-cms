#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std::string_literals;

#include <cmark.h>

std::string getUsageText(std::string const& cmd_name)
{
  return "Usage:\n"s
    + "    "s + cmd_name + " in_file.md [out_file.html]\n"s;
}

std::unordered_map<cmark_event_type, std::string> const CmarkEventNames = {
  { CMARK_EVENT_NONE, "NONE"s },
  { CMARK_EVENT_DONE, "DONE"s },
  { CMARK_EVENT_ENTER, "ENTER"s },
  { CMARK_EVENT_EXIT, "EXIT"s },
};

int main(int argc, char const** argv)
{
  if (argc < 2) {
    std::cerr << argv[0] << ": Markdown ファイルを指定してください。" << std::endl;
    std::cerr << getUsageText(argv[0]) << std::endl;
    return 1;
  }
  FILE * fin = fopen(argv[1], "rb");
  if (!fin) {
    std::cerr << argv[0] << ": Markdown ファイル " << argv[1] << " を開けません。" << std::endl;
    std::cerr << getUsageText(argv[0]) << std::endl;
    return 1;
  }
  cmark_node * doc = cmark_parse_file(fin, CMARK_OPT_VALIDATE_UTF8);

  cmark_event_type ev_type;
  cmark_iter * iter = cmark_iter_new(doc);
  while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
    cmark_node *cur = cmark_iter_get_node(iter);
    std::cout << cmark_node_get_type_string(cur)
      << ": event = "s << CmarkEventNames.at(ev_type)
      << " / pos = ("s << cmark_node_get_start_line(cur) << ","s << cmark_node_get_start_column(cur)
      << "-"s << cmark_node_get_end_line(cur) << ","s << cmark_node_get_end_column(cur)
      << ")"s << std::endl;
    if (cmark_node_get_type(cur) == CMARK_NODE_CODE_BLOCK) {
      std::cout << "fence info = '"s << cmark_node_get_fence_info(cur) << "'"s << std::endl;
    }
    char const* literal = cmark_node_get_literal(cur);
    if (literal) {
      std::cout << "-----\n"s << literal << "\n-----" << std::endl;
    }
  }
  cmark_iter_free(iter);
  cmark_node_free(doc);

  return 0;
}
