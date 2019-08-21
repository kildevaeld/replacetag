use replacetag_sys as sys;
use std::fmt;
pub struct ReplacerString {
    inner: *mut sys::fp_buffer_t,
}

impl ReplacerString {
    fn new() -> ReplacerString {
        ReplacerString {
            inner: unsafe { sys::fp_buffer_new() },
        }
    }
}

impl fmt::Display for ReplacerString {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        unsafe {
            let c = sys::fp_buffer_to_string(self.inner);
            let cs = std::ffi::CStr::from_ptr(c);
            write!(f, "{}", cs.to_string_lossy())?;
    
        }

        Ok(())
    }
}

impl Drop for ReplacerString {
    fn drop(&mut self) {
        unsafe {
            sys::fp_buffer_free(self.inner);
        }
    }
}

pub struct Pattern {
    open: char,
    close: char,
    pattern: Option<String>,
    replacer: Box<Fn(&str) -> Option<String>>,
}

impl Pattern {
    pub fn new<F: (Fn(&str) -> Option<String>) + 'static>(
        open: char,
        close: char,
        replacer: F,
    ) -> Pattern {
        Pattern {
            open,
            close,
            pattern: None,
            replacer: Box::new(replacer),
        }
    }

    pub fn pattern(&mut self, p: &str) -> &mut Self {
        self.pattern = Some(String::from(p));
        self
    }

    fn replace(&self, data: &str) -> Option<String> {
        (self.replacer)(data)
    }
}

pub struct Replacer {
    //inner: *mut sys::
    inner: *mut sys::fp_format_t,
}

impl Replacer {
    pub fn new() -> Replacer {
        Replacer {
            inner: unsafe { sys::fp_format_new() },
        }
    }

    pub fn replace<'a>(&self, input: &'a str) -> ReplacerString {
        let m = ReplacerString::new();
        unsafe {
            sys::fp_format_parse(
                self.inner,
                input.as_ptr() as *const i8,
                input.len() as i32,
                m.inner,
            );
        }
        m
    }

    pub fn add_pattern(&mut self, pattern: Pattern) {
        unsafe {
            let p = sys::fp_pattern_new(pattern.open as i8, pattern.close as i8, Some(_c_replace));
            if let Some(pat) = &pattern.pattern {
                sys::fp_pattern_set_pattern(
                    p,
                    std::ffi::CStr::from_bytes_with_nul(format!("{}\0", pat).as_bytes())
                        .expect("pattern")
                        .as_ptr(),
                );
            }
            let b = Box::new(pattern);
            sys::fp_pattern_set_release(p, Some(_c_replace_free));
            sys::fp_pattern_set_data(p, Box::into_raw(b) as *mut std::ffi::c_void);
            sys::fp_format_pattern_add(self.inner, p);
        }
    }
}

unsafe extern "C" fn _c_replace(
    input: *const i8,
    len: i32,
    chunk: *mut replacetag_sys::fp_chunk_s,
    data: *mut std::ffi::c_void,
) -> bool {
    let b = Box::from_raw(data as *mut Pattern);

    let data = std::slice::from_raw_parts(input as *const u8, len as usize);

    let mut ret = false;

    match std::str::from_utf8(data) {
        Ok(s) => match b.as_ref().replace(s) {
            Some(m) => {
                let len = m.len();
                let c = std::ffi::CString::new(m).unwrap();
                (*chunk).data = std::ffi::CString::into_raw(c) as *mut i8;
                (*chunk).len = (len + 1) as i32;
                ret = true;
            }
            None => {}
        },
        Err(_) => {}
    }

    std::mem::forget(b);

    return ret;
}

unsafe extern "C" fn _c_replace_free(
    chunk: *mut replacetag_sys::fp_chunk_s,
    data: *mut std::ffi::c_void,
) {
    let c = std::ffi::CString::from_raw((*chunk).data);
    drop(c);

    let b = Box::from_raw(data as *mut Pattern);
    drop(b);
}
