import commonmark
from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import HtmlFormatter
from pygments.util import ClassNotFound as LexerClassNotFound
import os, shutil, argparse

class CmdOption:
    def __init__(self):
        parser = argparse.ArgumentParser(description = "CommonMark to HTML converter.")
        parser.add_argument("in_dir", type = self.check_valid_dir)
        parser.add_argument("out_dir", nargs = "?", type = self.dir_chop)
        args = parser.parse_args()

        self.in_dir = args.in_dir
        self.out_dir = args.out_dir or os.getcwd()

    @staticmethod
    def dir_chop(val):
        val = val[:-1] if val.endswith('/') else val
        if not val:
            raise argparse.ArgumentTypeError("ルートディレクトリは指定できません (このエラーは回避可能ですが、通常そうすべきではありません…)")
        return val

    @staticmethod
    def check_valid_dir(val):
        if not os.path.isdir(val):
            raise argparse.ArgumentTypeError(f"ディレクトリ {val} が見つかりません")
        return CmdOption.dir_chop(val)

class CommonMark2HTML:
    def __init__(self):
        self.parser = commonmark.Parser()
        self.renderer = commonmark.HtmlRenderer()
        self.hl_fmtr = HtmlFormatter()
        self.doc = None

    def load(self, fh):
        md = fh.read()
        doc = self.parser.parse(md)
        for cur, entering in doc.walker():
            if cur.t == "code_block":
                language = cur.info
                try:
                    lexer = get_lexer_by_name(language)
                except LexerClassNotFound:
                    # 言語指定がない、または未対応の言語である場合は無視し、コードブロックのままにしておく
                    continue
                code_html = highlight(cur.literal, lexer, self.hl_fmtr)
                node = commonmark.node.Node("html_block", cur.sourcepos)
                node.literal = code_html
                cur.insert_before(node)
                cur.unlink()
        self.doc = doc

        return self

    def __str__(self):
        if not self.doc:
            return ""
        return self.renderer.render(self.doc)

    def get_style_defs(self):
        return self.hl_fmtr.get_style_defs('.highlight')

if __name__ == "__main__":
    cmdopt = CmdOption()
    os.makedirs(cmdopt.out_dir, exist_ok = True)
    conv = CommonMark2HTML()
    with open(os.path.join(cmdopt.out_dir, "style.css"), "w") as fout:
        fout.write(conv.get_style_defs())
    for cur, _, files in os.walk(cmdopt.in_dir):
        dir_offset = cur[len(cmdopt.in_dir) + 1:]
        dir_level = dir_offset.count('/') + 1 if dir_offset else 0
        os.makedirs(os.path.join(cmdopt.out_dir, dir_offset), exist_ok = True)
        for file in files:
            basename, ext = os.path.splitext(file)
            if ext.lower() == ".md":
                with open(os.path.join(cur, file), "r") as fin:
                    conv.load(fin)
                with open(os.path.join(cmdopt.out_dir, dir_offset, f"{basename}.html"), "w") as fout:
                    fout.write(f"""<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>{os.path.join(cur, file)}</title>
  <link rel="stylesheet" href="{''.join(['../'] * dir_level)}style.css">
</head>
<body>
{conv}
</body>
</html>
""")
            else:
                shutil.copy(os.path.join(cur, file), os.path.join(cmdopt.out_dir, dir_offset))
