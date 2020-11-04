use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::path::Path;

pub enum SourceHighlightImpl {}

extern "C"
{
    fn SourceHighlight_new(output_lang: *const c_char, data_dir: *const c_char)
        -> *mut SourceHighlightImpl;
    fn SourceHighlight_delete(shi: *mut SourceHighlightImpl);
    fn SourceHighlight_setLanguage(shi: *mut SourceHighlightImpl, language: *const c_char)
        -> bool;
    fn SourceHighlight_highlight(shi: *mut SourceHighlightImpl, src: *const c_char)
        -> *const c_char;
}

pub struct SourceHighlight {
    raw: *mut SourceHighlightImpl,
}

impl SourceHighlight {
    pub fn new<T: AsRef<str>, P: AsRef<Path>>(output_lang: T, data_dir: Option<P>) -> Self {
        let output_lang = CString::new(output_lang.as_ref().clone());
        let data_dir = CString::new(if let Some(dd) = data_dir {
            if dd.as_ref().is_dir() { dd.as_ref().to_str().unwrap() }
            else { "" }
        } else { "" });

        SourceHighlight{
            raw: unsafe { SourceHighlight_new(output_lang.as_ptr(), data_dir.as_ptr()) },
        }
    }
}
