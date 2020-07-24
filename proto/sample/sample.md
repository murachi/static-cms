# markdown ファイルのサンプル

markdown ファイルのサンプルです。
改行を含むテキストはどう扱われるか。

```
コードブロックのサンプルです。
プレーンテキストなので言語指定は特にありません。
```

```perl
#!/usr/bin/perl
# Perl5 スクリプトのサンプルです。
use strict;
use warnings;

package Hoge;

sub func {
  my $self = shift;
  # 特に何もしない
  return $self;
}
```

```cpp
/**
  @file hoge.hpp
  @brief ほげほげプログラム
  @author T.MURACHI
*/
#include <string>
#include <ostream>
using namespace std::string_literals;

namespace hoge {
  /// ほげほげクラス
  class Hoge {
    std::string text;

  public:
    /// コンストラクタ
    Hoge() : text{"ほげほげ"s} {}
    /// コンストラクタ
    explicit Hoge(std::string const& tx) : text{tx} {}
    /// コンストラクタ
    explicit Hoge(std::string && tx) : text{tx} {}

    /** ほげほげな値を返す
    @return ほげほげな値
    */
    std::string const& getText() const { return text; }
  }

  /** ほげほげクラス用出力ストリーム演算子
  @param[out]   ostrm 出力先ストリームオブジェクト
  @param[in]    hoge  ほげほげクラスオブジェクト
  @return ostrm を返す
  inline std::ostream & operator<<(std::ostream & ostrm, Hoge const& hoge) { return ostrm << hoge.getText(); }
}
```

## HTML で何か書くテスト

<b>太字</b>、<i>斜体</i>、<s>打ち消し線</s>。

<address><a href="mailto:email-addr@example.jp">email-addr@example.jp</a></address>

<pre><code class="py">import os

# スクリプトファイルが存在する場所
base_dir = os.path.join(os.path.dirname(__file__))
</code></pre>
