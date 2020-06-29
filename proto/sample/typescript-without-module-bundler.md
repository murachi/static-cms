# クライアントサイドスクリプトをモジュールバンドラを使わずに TypeScript で記述する

## 動機

TypeScript を使ってクライアントサイドスクリプトを書きたいんだけど、共用関数をファイルを分けて書こうとすると、トランスパイラはモジュールを扱うためのコードを生成する。実際の Web ブラウザは (IE や Edge を捨てて ES2015 を採用するのでない限り) これらのコードを理解できないため、これをクライアントサイドで動かすためには通常何らかのモジュールバンドラを併用する必要がある。

しかしながら、 AMD 方式で [RequireJS](https://requirejs.org/) を利用するにせよ、 CommonJS 方式で [Browserify](http://browserify.org/) を利用するにせよ、あるいは方式とか関係なく [webpack](https://webpack.js.org/) を利用するにせよ、生成される JavaScript コードは文字列として `eval()` されるかバンドラによってコンパイルされて難読化されてしまうため、ブラウザ上ではまともにデバッグできなくなる。 webpack ではデバッグサーバーを立てることで TypeScript コードに対するデバッグができるが、その場合サーバーサイドの挙動とどう接続するかなど色々と工夫が必要になり、設定も膨れ上がってしまう。

それなりの規模の Web アプリケーションを実装するならばともかく、ちょっとしたものを作るのにはあまりにも大げさすぎるし、そうでなくても開発初期の動作パフォーマンスを気にかけるべきでない時期からこれを気にかけながら開発を進めるのはとてもつらい。でもだからといってこれを避けるために TypeScript ごと諦めるのはとてももったいない。

そんなわけで、 TypeScript は使うけどモジュールバンドラはとりあえず使わずに実装をすすめる方法を見つけたので以下にメモしておきます。役に立つシチュエーションはとても限られていますが… (特に React その他のフレームワークを使うんであればあんまり役に立たないかも… (´・_・`)

## 想定

- とりあえず今回サーバーサイドに Python + flask を使っていたのでそれを前提に。それ以外でも応用のされ方はそんなに変わらないと思う。
- Node.js を導入し、 npm でコードベースに typescript をインストールするものとする。実際には sass もインストールしたので合わせてメモっときます。

## やるべきこと

- `tsc` のコンパイラオプション `--module` は `CommonJS` を指定する。
  - `tsconfig.json` は概ね以下の通り。重要そうな部分だけ抜き出したけど色々と足りなかったらごめんなさい(´・_・`)
    ```json
    {
      "compilerOptions": {
        "target": "es5"
        , "module": "commonjs"
        , "lib": ["es5", "dom"]
        , "outDir": "./src/static"
      }
      , "compileOnSave": true
      , "filesGlob": [
        "./ts/*.ts"
      ]
    }
    ```
    - `compileOnSave` を `true` にしたのは Atom で [atom-typescript](https://atom.io/packages/atom-typescript) を使って TypeScript コードを保存した時点でコンパイルが走るようにするため。必須ではないです。
    - `--lib` に指定したのはブラウザ上で標準的に使えるものと想定する API ですね。ここでは EcmaScript5 で規定されている各種 API と DOM が使える環境を想定しています。
  - 実際にトランスパイルした場合の例をいくつか。
    - `validator.ts`
      ```ts
      export namespace validator {
        export function validateNameToken(val: string): boolean {
          return /^[a-z]\w*$/i.test(val);
        }
      };
      ```
      `validator.js`
      ```js
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var validator;
      (function (validator) {
          function validateNameToken(val) {
              return /^[a-z]\w*$/i.test(val);
          }
          validator.validateNameToken = validateNameToken;
      })(validator = exports.validator || (exports.validator = {}));
      ;
      ```
      `exports` という変数が存在する想定で、名前空間 `validator` に各種定義を代入している。
      なお、 webpack を使う場合、 `export namespace` ではなく `export module` としないと何故かビルドが通らなかったりする。
    - `edit-account.ts`
      ```ts
      import { validator } from "./validator";

      window.addEventListener("DOMContentLoaded", function() {
        let account_form: HTMLFormElement = document.forms["account"];
        account_form.addEventListener("submit", function(ev) {
          // validation
          if (!validator.validateNameToken(this.elements["id"].value)) {
            window.alert("ユーザーID は半角英字で始まり半角英数字とアンダーバー _ のみで構成される名前にしてください。");
            ev.stopPropagation();
            ev.preventDefault();
          }
        });
      });
      ```
      `edit-account.js`
      ```js
      "use strict";
      Object.defineProperty(exports, "__esModule", { value: true });
      var validator_1 = require("./validator");
      window.addEventListener("DOMContentLoaded", function () {
          var account_form = document.forms["account"];
          account_form.addEventListener("submit", function (ev) {
              // validation
              if (!validator_1.validator.validateNameToken(this.elements["id"].value)) {
                  window.alert("ユーザーID は半角英字で始まり半角英数字とアンダーバー _ のみで構成される名前にしてください。");
                  ev.stopPropagation();
                  ev.preventDefault();
              }
          });
      });
      ```
      `require()` という関数が存在するものとして呼んでいる。 `require()` が返したオブジェクトから、名前空間 `validator` にアクセスできるという想定でコードが書かれている。なお、 TypeScript 側で指定している、実際に使用する予定の名前の一覧は、特に考慮されていない模様。

      ちなみに webpack を使用する場合、 `import` 文の `from` 句に渡す相対パス名には拡張子 `.js` を書き加える必要がある。そうしないとトランスパイルされた `require()` 関数が存在しないファイルを参照するとみなされるかららしい。 TypeScript としては `from` 句に渡す相対パス名に拡張子が含まれるのは illegal なのでこれもまた悩ましいところ。
- トランスパイルされた JavaScript コードを使用する HTML が以下のように出力されるようにする。
  ```html
  <!doctype html>
  <html lang="ja">
  <head>
    <meta charset="utf-8">
    <title>HTML 出力例</title>
    <link rel="stylesheet" href="/static/main.css">
    <script><!--
      var exports = {};
      function require(mod_path) {
        return exports;
      }
    --></script>
    <script src="/static/validator.js"></script>
    <script src="/static/edit-account.js"></script>
  </head>
  <body>

  ...

  </body>
  </html>
  ```
    - flask (テンプレートエンジンに jinja2) を使用する場合のテンプレート記述例:
      - `base.html`
        ```html
        <!doctype html>
        <html lang="ja">
        <head>
          <meta charset="utf-8">
          <title>{% block title %}{% endblock %} - myApp</title>
          <link rel="stylesheet" href="/static/main.css">
          <script><!--
            var exports = {};
            function require(mod_path) {
              return exports;
            }
          --></script>
          {% block additional_head %}{% endblock %}
        </head>
        <body>
          <header>
            <h1><a href="/">myApp</a></h1>
            {% block header %}{% endblock %}
          </header>
          <section class="main">
            <h1>{{ self.title() }}</h1>
            {% block main %}{% endblock %}
          </section>
        </body>
        </html>
        ```
      - `edit-account.html`
        ```html
        {% extends "base.html" %}
        {% block title %}アカウント{% if is_new %}新規登録{% else %}編集{% endif %}{% endblock %}
        {% block additional_head %}
        <script src="/static/validator.js"></script>
        <script src="/static/edit-account.js"></script>
        {% endblock %}
        {% block main %}
        <section class="edit account">
          <form target="/admin/post-account" method="POST" name="account">

        ...(略)

        {% endblock %}
        ```

## 考え方

CommonJS は Node.js のモジュールシステムに準拠する方法らしい。トランスパイル例で示した通り、この方法では以下のような記述の JavaScript コードが生成される。

- `export` するシンボルは全て `exports` 変数のメンバーとして格納される。
  - 名前空間で括ったシンボルは名前空間の変数のメンバーとして格納されるので、名前空間自体も `export` する必要がある。
- `require()` メソッドは `import` しようとしているオブジェクトがメンバーとして登録されているオブジェクトを返す。引数にはファイルパスを受け取っているが、これが何を解決しようとしているかは不明。

そこで、

- `require()` メソッドが返すオブジェクトは、 `import` しようとしているオブジェクトがメンバーとして登録されていればよい。であれば、 `exports` そのものを返せばよいはず。
- `tsc` が生成する JavaScript コードは `exports` オブジェクトが既に存在する前提で書き出されるが、 `exports` がどういう値であるかは (オブジェクトであること以外は) 特に規定されていないので、とりあえず事前に空オブジェクト `{}` を代入しておけば良さそう。

という考え方で HTML ページを生成するようにしたところ、とりあえずは動いてくれるようになったという次第。
