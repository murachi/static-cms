mod srchilite;

use std::error::Error;
use std::fs::File;
use std::{env, process};
use std::io::{BufReader, BufWriter};
use std::io::prelude::*;
use std::path::{Path, PathBuf};
use pulldown_cmark::{Parser, Options, Event, Tag, CodeBlockKind, CowStr};

use srchilite::SourceHighlight;

const HTML_PREFIX: &str = r#"<!doctype html>
<html lang="ja">
<head>
  <meta charset="utf-8">
  <title>cm2html by rust output sample</title>
  <link rel="stylesheet" href="syntect.css">
</head>
<body>
"#;
const HTML_SUFFIX: &str = r#"</body>
</html>"#;

fn main() -> Result<(), Box<dyn Error>> {
    let args: Vec<_> = env::args().collect();
    if args.len() <= 1 {
        eprintln!("Markdown ファイルを指定してください。");
        process::exit(1);
    }
    let mut contents = String::new();
    {
        let file = File::open(&args[1])?;
        let mut br = BufReader::new(file);
        br.read_to_string(&mut contents)?;
    }

    //let parser = Parser::new_ext(&contents, Options::ENABLE_STRIKETHROUGH);
    //for event in parser {
    //    println!("{:?}", event);
    //}

    let parser = Parser::new_ext(&contents, Options::ENABLE_STRIKETHROUGH);
    let mut highlighter = SourceHighlight::new("htmlcss.outlang", Option::<PathBuf>::None);
    let mut is_ready_highlight = false;
    let mut src = Vec::new();
    let parser = parser.map(|event| -> Vec<Event> { match event {
        Event::Start(ref tag) => {
            if let Tag::CodeBlock(ref cbkind) = tag {
                if let CodeBlockKind::Fenced(ref fence) = cbkind {
                    if let CowStr::Borrowed(ref fc_text) = fence {
                        is_ready_highlight = highlighter.set_language(fc_text);
                    }
                }
            }
            if is_ready_highlight {
                src.clear();
                vec![]
            }
            else { vec![event] }
        },
        Event::Text(text) if is_ready_highlight => {
            let mut text = text.into_string();
            if !text.ends_with('\n') {
                text.push('\n');
            }
            src.append(&mut text.into_bytes());
            vec![]
        },
        Event::End(_) => {
            if is_ready_highlight {
                is_ready_highlight = false;
                src.push(b'\0');
                if let Ok(html) = unsafe {
                    highlighter.highlight_without_nul_check(src.as_slice())
                } {
                    vec![Event::Html(CowStr::Boxed(html.into_boxed_str()))]
                }
                else {
                    // 変換結果に非 Unicode が混入していた場合。
                    // 通常はありえないが、仮にあった場合は元のコードブロックと同等のものに
                    // 戻しておく
                    src.pop();
                    let mut text = String::from_utf8_lossy(src.as_slice()).into_owned();
                    while text.ends_with('\n') { text.pop(); }
                    vec![
                        Event::Start(Tag::CodeBlock(CodeBlockKind::Fenced(CowStr::Borrowed("")))),
                        Event::Text(CowStr::Boxed(text.into_boxed_str())),
                        Event::End(Tag::CodeBlock(CodeBlockKind::Fenced(CowStr::Borrowed("")))),
                    ]
                }
            }
            else { vec![event] }
        },
        _ => vec![event],
    }}).flatten();

    let mut html_output = String::new();
    pulldown_cmark::html::push_html(&mut html_output, parser);

    if args.len() <= 2 {
        print!("{}{}{}", HTML_PREFIX, html_output, HTML_SUFFIX);
    }
    else {
        let path = Path::new(&args[2]);
        let html_file = File::create(path)?;
        let mut bw = BufWriter::new(html_file);
        write!(bw, "{}{}{}", HTML_PREFIX, html_output, HTML_SUFFIX)?;
    }


    Ok(())
}
