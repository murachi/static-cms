"use strict";

const fs = require("fs");
const commonmark = require("commonmark");
const hljs = require("highlight.js");

function CommonMark2HTML() {
  this.cmParser = new commonmark.Parser();
  this.cmWriter = new commonmark.HtmlRenderer();
}

CommonMark2HTML.prototype.load = function(file) {
  const md_src = fs.readFileSync(file);
  this.doc = this.cmParser.parse(md_src.toString());

  const walker = this.doc.walker();
  let item;
  while (item = walker.next()) {
    if (item.node.type !== "code_block") continue;
    const cur = item.node;
    let html;
    try {
      //TODO: フェンス情報の取扱については別途考慮すべき
      html = hljs.highlight(cur.info, cur.literal).value;
    }
    catch (ex) {
      // 未対応の言語の場合、コードブロックは手を付けない
      if (/^Error: Unknown language/.test(ex.toString())) continue;
      else throw ex;
    }
    const node = new commonmark.Node("html_block", cur.sourcepos);
    node.literal = '<pre class="highlight"><code>' + html + '</code></pre>';
    cur.insertBefore(node);
    cur.unlink();
  }
}

CommonMark2HTML.prototype.toString = function() {
  return this.cmWriter.render(this.doc);
}

function usage(err_msg) {
  if (!!err_msg) {
    console.error(err_msg);
  }
  console.info("usage: node cm2html.js in_dir out_dir");
  process.exit(!!err_msg ? 1 : 0);
}

if (process.argv.length != 4) {
  usage("引数が正しくありません。");
}
const [,,indir, outdir] = process.argv;
if (!fs.existsSync(indir) || !fs.statSync(indir).isDirectory()) {
  usage(`'$indir' はディレクトリではありません。`);
}
if (!fs.existsSync(outdir)) {
  try {
    fs.mkdirSync(outdir, { recursive: true })
  }
  catch (ex) {
    usage(`'$outdir' ディレクトリの作成に失敗しました。`);
  }
}
else {
  if (!fs.statSync(outdir).isDirectory()) {
    usage(`'$outdir' はディレクトリではありません。`);
  }
}

const parser = new commonmark.Parser();
const md = fs.readFileSync(process.argv[2]);
const doc = parser.parse(md.toString());

const walker = doc.walker();
let item;
while (item = walker.next()) {
  if (item.node.type !== "code_block") continue;
  const cur = item.node;
  let html;
  try {
    html = hljs.highlight(cur.info, cur.literal).value;
  }
  catch (ex) {
    if (/^Error: Unknown language/.test(ex.toString())) continue;
    else throw ex;
  }
  const node = new commonmark.Node("html_block", cur.sourcepos);
  node.literal = '<pre class="highlight"><code>' + html + '</code></pre>';
  cur.insertBefore(node);
  cur.unlink();
}

const renderer = new commonmark.HtmlRenderer();
const html_file = process.argv.length >= 4 ? process.argv[3] : process.argv[2] + ".html";
fs.writeFileSync(html_file, renderer.render(doc));
