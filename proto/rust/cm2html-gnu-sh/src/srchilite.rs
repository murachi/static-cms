use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::path::Path;
use std::error::Error;

pub enum SourceHighlightImpl {}

#[link(name="srchilite-wrapper", kind="static")]
extern {
    fn SourceHighlight_new(output_lang: *const c_char, data_dir: *const c_char)
        -> *mut SourceHighlightImpl;
    fn SourceHighlight_delete(shi: *mut SourceHighlightImpl);
    fn SourceHighlight_setLanguage(shi: *mut SourceHighlightImpl, language: *const c_char)
        -> bool;
    fn SourceHighlight_highlight(shi: *mut SourceHighlightImpl, src: *const c_char)
        -> *mut c_char;
}

pub struct SourceHighlight {
    raw: *mut SourceHighlightImpl,
}

impl SourceHighlight {
    pub fn new<T: AsRef<str>, P: AsRef<Path>>(output_lang: T, data_dir: Option<P>) -> Self {
        let output_lang = CString::new(output_lang.as_ref().clone()).unwrap();
        let data_dir = CString::new(if let Some(ref dd) = data_dir {
            if dd.as_ref().is_dir() { dd.as_ref().to_str().unwrap().clone() }
            else { "/usr/share/source-highlight" }
        } else { "/usr/share/source-highlight" }).unwrap();

        SourceHighlight{
            raw: unsafe { SourceHighlight_new(output_lang.as_ptr(), data_dir.as_ptr()) },
        }
    }

    #[inline]
    pub fn set_language(&mut self, language: &str) -> bool {
        let language = CString::new(language.clone()).unwrap();
        unsafe { SourceHighlight_setLanguage(self.raw, language.as_ptr()) }
    }

    #[inline]
    pub fn highlight(&mut self, src: &[u8]) -> Result<String, Box<dyn Error>> {
        let src = CStr::from_bytes_with_nul(src)?;
        Ok(unsafe { CString::from_raw(SourceHighlight_highlight(self.raw, src.as_ptr())) }
            .into_string()?)
    }

    #[inline]
    pub unsafe fn highlight_without_nul_check(&mut self, src: &[u8])
        -> Result<String, Box<dyn Error>>
    {
        let src = CStr::from_bytes_with_nul_unchecked(src);
        Ok(CString::from_raw(SourceHighlight_highlight(self.raw, src.as_ptr())).into_string()?)
    }
}

impl Drop for SourceHighlight {
    fn drop(&mut self) {
        unsafe {
            SourceHighlight_delete(self.raw);
        }
    }
}
