# static-cms - 提案メモ

## システム概要

github に登録したリポジトリ上で Markdown ファイルを追加・変更すると、 master ブランチに push したタイミングで実際の Web サイト上に記事が追加・更新されるようなシステムを作りたい。

## ゴール (解決したいこと)

- テキストエディタベースで Markdown 書いて github に push したらブログが公開・更新されるようにしたい。
- 過去のブログや日記の類を集約したい。

## 具体的な機能

### github 連携

github 等の履歴管理機能を伴うツールやサービスと連携し、 master ブランチ (仮に Subversion 対応するならメイントランク) への変更を検知したら Web サーバーへ通知する。 Web サーバー側は通知を受けたらリポジトリを fetch &amp; pull し、変更箇所を HTML 化して Web 公開用のファイル構成に適用する。

- 簡単のため、当面は github の使用のみを対象とする (将来的には GitLab やその他のツール・サービス、 git 単体などにも対応したい)。
- github の場合、 [Actions](https://github.com/marketplace?type=actions) を使うことでリポジトリへの操作をフックし、様々な処理を実行することができるので、これを利用する。
  - [GitHubの新機能「GitHub Actions」で試すCI/CD (さくらのナレッジ)](https://knowledge.sakura.ad.jp/23478/)
- Web サーバー側では記事更新の通知を受け付けて一連の処理を実行する仕組みを用意する。この仕組みは、それこそ Web API として実装すれば良いと思う。 DoS 攻撃対策のために認証には気をつける必要がある。
- github への設定については設定手順をドキュメントに尽くせば良い (Actions 用のスクリプトを用意したほうが良さそうなら用意する)。サーバーへの設定についてはデプロイ環境を整える必要がある (セットアップスクリプトを用意するか、または docker コンテナを用意する)。

### 記事変換

更新通知を受けた Web サーバーはリポジトリを fetch &amp; pull し、変更箇所を検知して、必要に応じて HTML への変換を行い、 Web 公開用のドキュメントファイル構成に反映 (コピー) する。

- ファイルの削除やファイル名の変更にも対応する。
- 変換対象は (デフォルトでは) Markdown (`*.md`) と HTML (`*.htm`, `*.html`) とする。
  - いずれもテンプレート HTML 中に (Markdown であれば HTML に変換後の内容を) 組み込んだファイルを生成し、それをコピーする。
  - HTML ファイルの場合は同じファイル名で、 Markdown ファイルの場合は拡張子 `.md` を `.html` に置き換えたファイル名でコピーする。
  - Markdown ファイルのコードブロックは fence に言語やファイル名の記載があれば、該当する言語の文法でハイライトを行ったり、ファイル名を併記したりといった処理を加える。
    - HTML ファイルの `<pre>` タグに特定の `class` 属性指定でもハイライトに対応すべきか検討する。
  - Markdown ファイルの見出し要素や HTML の `<h*>` タグ (`*` は数字) を使って目次 (TOC) を作れるようにする。
    - 実際に作るかどうかは、テンプレートにプレースホルダーを用意するか、または文中で専用タグを埋め込むなどのシチュエーションに準ずる。
- それ以外のファイルは、デフォルトではそのままコピーする。
  - ピリオド `.` で始まる名前のファイルは処理しない。
- 特定の名前のファイル (ピリオド `.` で始まるファイル名が望ましい) を、そのディレクトリ以下に影響する設定ファイルとなるようにする。
- ディレクトリ構成はそのまま踏襲する。但し、設定ファイルによってサブディレクトリ以下のコピー先を変更できるようにする。
- コピー元 (リポジトリをチェックアウトするディレクトリ) とコピー先 (Web サービスのドキュメントルートとなるディレクトリ) の関係は特定の規約に基づくのではなく、単に設定ファイルによって設定させるようにする。互いのルートの設定はセットアップ時に確定するようにする。

### 閲覧者コメント

閲覧者からのコメントを受け付ける。コメントは記事 URL に関連する別 URL にてフォームデータとして受け取り、最終的にはその内容を記事ページ内に反映する。

- CSRF に配慮する。具体的には、閲覧者がコメントを投稿する操作の後にセッションを開始し、投稿内容の確認を求めるようにする。
  - bot 対策のための機能については、追々検討する。
- トラックバックについては当面は対応しない。
- spam 対策のためにコメントの審査やフィルタリングができるような機能を用意する。
- コメントの記事ページへの反映は、設定に基づく定期的または同期的なタイミングにより、コメント情報の更新差分が認められる記事に対する通常の記事更新処理の中で行う。
- コメントは Markdown 形式にて受け付けるものとする。 HTML は安全を配慮して制限を設ける。
  - 制限に引っかかる箇所は、単にテキストとして表示するようにする。
