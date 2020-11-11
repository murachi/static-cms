fn main() {
    cc::Build::new()
        .cpp(true)
        .file("src/c/srchilite-wrapper.cpp")
        .compile("libsrchilite-wrapper.a");
    pkg_config::probe_library("source-highlight").unwrap();
}
