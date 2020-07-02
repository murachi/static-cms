const fs = require("fs");
const commonmark = require("commonmark");
const hljs = require("highlight.js");

if (process.argv.length < 3) {
  // usage
  process.exit(1);
}

const parser = commonmark.Parser();
