use std::sync::Arc;
use std::pin::Pin;
use std::mem;

use super::ffi;

pub struct InnerDescriptor {
    internal: cxx::UniquePtr<ffi::RustyDescriptor>,
}

impl InnerDescriptor {
    pub(crate) fn new(wrapper: &mut ffi::RustyDescriptorWrapper) -> Pin<Box<Self>> {
        let this = Self {
            internal: cxx::UniquePtr::<ffi::RustyDescriptor>::null(),
        };

        let mut this_boxed = Box::pin(this);
        mem::swap(&mut this_boxed.internal, &mut wrapper.internal);

        return this_boxed;
    }

    pub fn uuid(&self) -> String {
        return self.internal.uuid();
    }

    pub fn initialized(&self) -> bool {
        return self.internal.initialized();
    }
}

unsafe impl Sync for InnerDescriptor {}
unsafe impl Send for InnerDescriptor {}

#[derive(Clone)]
pub struct Descriptor {
    inner: Arc<Pin<Box<InnerDescriptor>>>,
}

impl Descriptor {
    // TODO: Decide how to implement `new`

    pub fn uuid(&self) -> String {
        return self.inner.uuid();
    }

    pub fn initialized(&self) -> bool {
        return self.inner.initialized();
    }
}


impl From<Pin<Box<InnerDescriptor>>> for Descriptor {
    fn from(descriptor: Pin<Box<InnerDescriptor>>) -> Self {
        return Descriptor {
            inner: Arc::new(descriptor),
        };
    }
}

unsafe impl Send for Descriptor {}
unsafe impl Sync for Descriptor {}
