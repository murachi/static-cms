# crawl_grammars.py - tree-sitter 用文法定義クローラ
import os, re, json, base64, argparse
import http.client

EXCLUDE_SRC_FILE = ('binding.cc',)
HEADER_MISSING = {
    ('razor', 'src'): ['c-sharp', 'src'],
}

class CrawlerException(Exception):
    pass

class Crawler:
    EXCLUDE_LANGS = ('cli', 'razor', 'ocaml')
    SRC_PLACES = {
        'ocaml': (('interface', 'src'), ('ocaml', 'src')),
    }

    def __init__(self, token):
        self.token = token
        self.langs = None

    def request_API(self, url, method = "GET", post_data = None):
        con = http.client.HTTPSConnection("api.github.com")
        body = json.JSONEncoder().encode(post_data) if post_data is not None else None
        con.request(method, url, body = body,
            headers = {
                "User-Agent": "crawl_grammars.py",
                "Authorization": f"token {self.token}",
            })
        res = con.getresponse()
        #TODO: res.status != 200 の場合の処理内容を検討
        return json.JSONDecoder().decode(res.read().decode("utf-8"))

    def download_all_grammars(self):
        for lang in self.pickup_langs():
            print(f"downloading {lang} grammar...")
            self.download_grammar(lang)

    def pickup_langs(self):
        if self.langs:
            return self.langs

        data = self.request_API("/users/tree-sitter/repos")
        self.langs = [
            lang for lang
            in (n["name"][12:] for n in data if n["name"].startswith("tree-sitter-"))
            if lang not in self.EXCLUDE_LANGS
        ]
        return self.langs

    def download_grammar(self, lang):
        src_root_dir = os.path.join(os.path.dirname(__file__), "src", "c", lang)
        os.makedirs(src_root_dir, exist_ok = True)

        repo = self.request_API(f"/repos/tree-sitter/tree-sitter-{lang}/branches/master")
        tree_root = self.request_API(f"/repos/tree-sitter/tree-sitter-{lang}/git/trees/{repo['commit']['sha']}")

        src_places = self.SRC_PLACES.get(lang, (('src',),))
        for place in src_places:
            dir = src_root_dir
            tree = tree_root
            for subdir in place:
                dir = os.path.join(dir, subdir)
                os.makedirs(dir, exist_ok = True)
                for item in tree["tree"]:
                    if item["path"] == subdir:
                        sha = item["sha"]
                        break
                else:
                    raise CrawlerException(
                        f"tree-sitter-{lang} リポジトリの master ブランチにて " +
                        f"{'/'.join(place)} ディレクトリが見つかりません")
                if subdir != 'src':
                    tree = self.request_API(f"/repos/tree-sitter/tree-sitter-{lang}/git/trees/{sha}")

            self.download_github_files(lang, dir, sha)

    def download_github_files(self, lang, dir, tree_sha):
        tree = self.request_API(f"/repos/tree-sitter/tree-sitter-{lang}/git/trees/{tree_sha}")
        items = [(n["path"], n["type"], n["sha"]) for n in tree["tree"]]
        for name, type, sha in items:
            if type == "tree":
                subdir = os.path.join(dir, name)
                os.makedirs(subdir, exist_ok = True)
                self.download_github_files(lang, subdir, sha)
            else:
                print(f"* {name} -> {os.path.join(dir, name)}")
                blob = self.request_API(f"/repos/tree-sitter/tree-sitter-{lang}/git/blobs/{sha}")
                with open(os.path.join(dir, name), "wb") as fout:
                    fout.write(base64.b64decode(blob["content"]))

def update_build_script(langs):
    with open(os.path.join(os.path.dirname(__file__), "build.rs"), "w") as fout:
        print("fn main() {", file = fout)
        c_src_root = os.path.join(os.path.dirname(__file__), "src", "c")
        for cur, _, files in os.walk(c_src_root):
            dir = cur
            ancs = []
            while dir != c_src_root:
                dir, t = os.path.split(dir)
                ancs.append(t)
            ancs.reverse()
            if not ancs or ancs[0] not in langs:
                continue
            include_dir = os.path.join(c_src_root, *HEADER_MISSING.get(tuple(ancs), ancs))
            libname_prefix = "lib{}".format('_'.join([n for n in ancs if n != 'src']))
            for file in (f for f in files if (f.endswith(".c") or f.endswith(".C")) and f not in EXCLUDE_SRC_FILE):
                print("    cc::Build::new()", file = fout)
                print(f'        .include("{include_dir}")', file = fout)
                print(f'        .file("{os.path.join(cur, file)}")', file = fout)
                print(f'        .compile("{libname_prefix}_{os.path.splitext(file)[0].replace(".", "_")}.a");',
                    file = fout)
            for file in (f for f in files if re.search(r"\.(?:cc|cpp|cxx|c\+\+)$", f, re.I) and f not in EXCLUDE_SRC_FILE):
                print("    cc::Build::new()", file = fout)
                print("        .cpp(true)", file = fout)
                print(f'        .include("{include_dir}")', file = fout)
                print(f'        .file("{os.path.join(cur, file)}")', file = fout)
                print(f'        .compile("{libname_prefix}_{os.path.splitext(file)[0].replace(".", "_")}.a");',
                    file = fout)
        print("}", file = fout)

def update_langman_module(langs):
    langs = [n.replace('-', '_') for n in langs]
    with open(os.path.join(os.path.dirname(__file__), "src", "langman.rs"), "w") as fout:
        print("use tree_sitter::Language;\n", file = fout)
        for lang in langs:
            print('extern "C" {{ fn tree_sitter_{}() -> Language; }}'.format(lang), file = fout)
        print('''
pub fn select_language<T: AsRef<str>>(lang_name: T) -> Option<Language> {
    match lang_name.as_ref() {''', file = fout)
        for lang in langs:
            print('        "{0}" => Some(unsafe {{ tree_sitter_{0}() }}),'.format(lang), file = fout)
        print('''        _ => None,
    }
}''', file = fout)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = "tree-sitter 用文法定義クローラ")
    parser.add_argument('-t', '--token', required = True, metavar = "認証用トークン")
    args = parser.parse_args()

    crawler = Crawler(args.token)
    crawler.download_all_grammars()

    update_build_script(crawler.pickup_langs())
    update_langman_module(crawler.pickup_langs())
