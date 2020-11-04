#include <vector>
#include <memory>
#include <string>
#include <srchilite/sourcehighlight.h>
#include <srchilite/langmap.h>
#include <cstring>

using namespace std::string_literals;

extern "C" {
  typedef struct {
    srchilite::SourceHighlight src_highlight,
    srchilite::LangMap lang_map,
    std::string lang_file,
    std::string result,
  } SourceHighlightImpl;

  SourceHighlightImpl* SourceHighlight_new(char const* output_lang, char const* data_dir)
  {
    std::string ol = output_lang && output_lang[0] ? output_lang : "html.outlang";
    std::string dd = data_dir && data_dir[0] ? data_dir : "";
    auto shi = new SourceHighlightImpl {
      srchilite::SourceHighlight{ol},
      srchilite::LangMap{dd, "lang.map"s},
    };
    shi->src_highlight.setDataDir(dd);
    return shi;
  }

  void SourceHighlight_delete(SourceHighlightImpl * shi)
  {
    delete shi;
  }

  bool SourceHighlight_setLanguage(SourceHighlightImpl * shi, char const* language)
  {
    shi->lang_file = shi->lang_map.getMappedFileName(language);
    return !shi->lang_file.empty();
  }

  char const* SourceHighlight_highlight(SourceHighlightImpl * shi, char const* src)
  {
    if (shi->lang_file.empty()) {
      return nullptr;
    }
    std::istringstream istrm{src};
    std::ostringstream ostrm;
    shi->src_highlight.highlight(istrm, ostrm, shi->lang_file);
    shi->result = ostrm.str();
    return shi->result.c_str();
  }
}
