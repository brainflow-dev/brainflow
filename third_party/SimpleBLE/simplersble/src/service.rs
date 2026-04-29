use std::sync::Arc;
use std::pin::Pin;
use std::mem;

use super::ffi;
use crate::characteristic::InnerCharacteristic;
use crate::characteristic::Characteristic;
pub struct InnerService {
    internal: cxx::UniquePtr<ffi::RustyService>,
}

impl InnerService {
    pub(crate) fn new(wrapper: &mut ffi::RustyServiceWrapper) -> Pin<Box<Self>> {
        let this = Self {
            internal: cxx::UniquePtr::<ffi::RustyService>::null(),
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

    pub fn data(&self) -> Vec<u8> {
        return self.internal.data();
    }

    pub fn characteristics(&self) -> Vec<Characteristic> {
        let mut characteristics = Vec::<Characteristic>::new();

        for characteristic_wrapper in self.internal.characteristics().iter_mut() {
            characteristics.push(InnerCharacteristic::new(characteristic_wrapper).into());
        }

        return characteristics;
    }
}

unsafe impl Sync for InnerService {}
unsafe impl Send for InnerService {}

#[derive(Clone)]
pub struct Service {
    inner: Arc<Pin<Box<InnerService>>>,
}

impl Service {
    pub fn uuid(&self) -> String {
        return self.inner.uuid();
    }

    pub fn initialized(&self) -> bool {
        return self.inner.initialized();
    }

    pub fn data(&self) -> Vec<u8> {
        return self.inner.data();
    }

    pub fn characteristics(&self) -> Vec<Characteristic> {
        return self.inner.characteristics();
    }

}

impl From<Pin<Box<InnerService>>> for Service {
    fn from(service: Pin<Box<InnerService>>) -> Self {
        return Service {
            inner: Arc::new(service),
        };
    }
}

unsafe impl Send for Service {}
unsafe impl Sync for Service {}
