use std::error::Error;
use std::fs::File;
use std::{env, process};
use std::io::{BufReader, BufWriter};
use std::io::prelude::*;
use std::path::Path;
use pulldown_cmark::{Parser, Options, Event, Tag, CodeBlockKind, CowStr};
use syntect::parsing::{SyntaxSet, SyntaxReference};
use syntect::highlighting::ThemeSet;
use syntect::html::{ClassedHTMLGenerator, ClassStyle};

const html_prefix: &str = r#"<!doctype html>
<html lang="ja">
<head>
  <meta charset="utf-8">
  <title>cm2html by rust output sample</title>
  <link rel="stylesheet" href="syntect.css">
</head>
<body>
"#;
const html_suffix: &str = r#"</body>
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

    let parser = Parser::new_ext(&contents, Options::ENABLE_STRIKETHROUGH);
    for event in parser {
        println!("{:?}", event);
    }

    let stx_set = SyntaxSet::load_defaults_newlines();
    //for stx in stx_set.syntaxes() {
    //    //println!("{:?}", stx);
    //    println!("{} ({}) - first line = {:?}, scope = {:?}",
    //        stx.name, stx.file_extensions.join(", "), stx.first_line_match, stx.scope);
    //}

    let parser = Parser::new_ext(&contents, Options::ENABLE_STRIKETHROUGH);
    let mut stx: Option<&SyntaxReference> = None;
    let parser = parser.map(|event| match event {
        Event::Start(ref tag) => {
            if let Tag::CodeBlock(ref cbkind) = tag {
                if let CodeBlockKind::Fenced(ref fence) = cbkind {
                    if let CowStr::Borrowed(ref fc_text) = fence {
                        stx = stx_set.find_syntax_by_token(*fc_text);
                    }
                }
            }
            if stx.is_some() { Event::SoftBreak }
            else { event }
        },
        Event::Text(text) if stx.is_some() => {
            let mut html_generator = ClassedHTMLGenerator::new_with_class_style(
                stx.unwrap(), &stx_set, ClassStyle::Spaced);
            for line in text.into_string().lines() {
                html_generator.parse_html_for_line(&line);
            }
            let html = format!("<pre class=\"src\">{}</pre>", html_generator.finalize());
            Event::Html(CowStr::Boxed(html.into_boxed_str()))
        },
        Event::End(_) if stx.is_some() => {
            stx = None;
            Event::SoftBreak
        },
        _ => event,
    });

    let mut html_output = String::new();
    pulldown_cmark::html::push_html(&mut html_output, parser);

    let theme_set = ThemeSet::load_defaults();
    let theme = theme_set.themes.get("InspiredGitHub").unwrap();
    let css = syntect::html::css_for_theme_with_class_style(&theme, ClassStyle::Spaced);

    if args.len() <= 2 {
        print!("{}{}{}", html_prefix, html_output, html_suffix);
        println!("\n--- CSS ---\n{}", css);
    }
    else {
        let path = Path::new(&args[2]);
        let html_file = File::create(path)?;
        let mut bw = BufWriter::new(html_file);
        write!(bw, "{}{}{}", html_prefix, html_output, html_suffix)?;
        let css_file = File::create(path.parent().unwrap().join("syntect.css"))?;
        bw = BufWriter::new(css_file);
        write!(bw, "{}", css)?;
    }


    Ok(())
}
