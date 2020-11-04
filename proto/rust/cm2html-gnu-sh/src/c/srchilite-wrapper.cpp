#include <vector>
#include <memory>
#include <string>
#include <srchilite/sourcehighlight.h>
#include <srchilite/langmap.h>

using namespace std::string_literals;

extern "C" {
  typedef struct {
    srchilite::SourceHighlight impl
  } SourceHighlightImpl;

  SourceHighlightImpl* SourceHighlight_new(char const* output_lang, char const* data_dir)
  {
    std::string ol = output_lang && output_lang[0] ? output_lang : "html.outlang";
    auto shi = new SourceHighlightImpl { srchilite::SourceHighlight{ol} };
    shi->impl.setDataDir(data_dir && data_dir[0] ? data_dir : "");
    return shi;
  }

  void SourceHighlight_delete(SourceHighlightImpl * shi)
  {
    delete shi;
  }


}
