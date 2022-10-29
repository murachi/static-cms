"use strict";

const fs = require("fs");
const commonmark = require("commonmark");
const refractor = require("refractor");
const toHTML = require("hast-util-to-html");

if (process.argv.length < 3) {
  console.error("Markdown ファイルを指定してください。");
  console.info("usage: node cm2html.js in_file.md [out_file.html]")
  process.exit(1);
}

const parser = new commonmark.Parser();
const md = fs.readFileSync(process.argv[2]);
const doc = parser.parse(md.toString());

const walker = doc.walker();
let item;
while (item = walker.next()) {
  if (item.node.type !== "code_block") continue;
  const cur = item.node;
  let tree = { type: "root", children: [] };
  try {
    tree.children = refractor.highlight(cur.literal, cur.info);
  }
  catch (ex) {
    if (/^Error: Unknown language/.test(ex.toString())) continue;
    else throw ex;
  }
  const node = new commonmark.Node("html_block", cur.sourcepos);
  node.literal = '<pre class="highlight"><code>' + toHTML(tree) + '</code></pre>';
  cur.insertBefore(node);
  cur.unlink();
}

const renderer = new commonmark.HtmlRenderer();
const html_file = process.argv.length >= 4 ? process.argv[3] : process.argv[2] + ".html";
fs.writeFileSync(html_file, renderer.render(doc));
