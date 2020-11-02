mod langman;

use std::error::Error;
use std::fs::File;
use std::{env, process};
use std::io::{BufReader, BufWriter};
use std::io::prelude::*;
use std::path::Path;
use pulldown_cmark::{Parser, Options, Event, Tag, CodeBlockKind, CowStr};
use tree_sitter::{self, Language};

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
    let mut lang_parser = tree_sitter::Parser::new();
    let mut language: Option<Language> = None;
    let mut src = String::new();
    let parser = parser.map(|event| -> Vec<Event> { match event {
        Event::Start(ref tag) => {
            if let Tag::CodeBlock(ref cbkind) = tag {
                if let CodeBlockKind::Fenced(ref fence) = cbkind {
                    if let CowStr::Borrowed(ref fc_text) = fence {
                        language = langman::select_language(fc_text);
                    }
                }
            }
            if language.is_some() {
                src = String::new();
                vec![]
            }
            else { vec![event] }
        },
        Event::Text(text) if language.is_some() => {
            for line in text.into_string().lines() {
                src.push_str(&format!("{}\n", line));
            }
            vec![]
        },
        Event::End(_) => {
            if let Some(language) = language.take() {
                let mut events: Vec<Event> = vec![Event::Start(Tag::List(None))];
                lang_parser.set_language(language).unwrap();
                let tree = lang_parser.parse(&src, None).unwrap();
                let mut cursor = tree.walk();
                'dig_tree: loop {
                    let node = cursor.node();
                    let text = format!("id={}, kind={}, start_position={}, end_position={}",
                        node.id(), node.kind(), node.start_position(), node.end_position());
                    //println!("{}", text);
                    events.append(&mut vec![
                        Event::Start(Tag::Item),
                        Event::Text(CowStr::Boxed(text.into_boxed_str())),
                        Event::End(Tag::Item),
                    ]);
                    if cursor.goto_first_child() {
                        events.push(Event::Start(Tag::List(None)));
                    }
                    else {
                        while !cursor.goto_next_sibling() {
                            events.push(Event::End(Tag::List(None)));
                            if !cursor.goto_parent() {
                                break 'dig_tree;
                            }
                        }
                    }
                }
                events
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
