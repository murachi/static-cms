import commonmark
from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import HtmlFormatter
from pygments.util import ClassNotFound as LexerClassNotFound
import argparse

class CmdOption:
    def __init__(self):
        parser = argparse.ArgumentParser(description = "CommonMark to HTML comverter.")
        parser.add_argument("in_file")
        parser.add_argument("out_file", nargs = "?")
        args = parser.parse_args()

        self.in_file = args.in_file
        self.out_file = args.out_file or args.in_file + ".html"

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

if __name__ == "__main__":
    cmdopt = CmdOption()
    conv = CommonMark2HTML()
    with open(cmdopt.in_file, "r") as fin:
        conv.load(fin)
    with open(cmdopt.out_file, "w") as fout:
        fout.write(f"{conv}")
