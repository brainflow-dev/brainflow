use std::sync::Arc;
use std::pin::Pin;
use std::mem;

use super::ffi;
use crate::descriptor::InnerDescriptor;
use crate::descriptor::Descriptor;
use crate::types::CharacteristicCapability;

pub struct InnerCharacteristic {
    internal: cxx::UniquePtr<ffi::RustyCharacteristic>,
}

impl InnerCharacteristic {
    pub(crate) fn new(wrapper: &mut ffi::RustyCharacteristicWrapper) -> Pin<Box<Self>> {
        let this = Self {
            internal: cxx::UniquePtr::<ffi::RustyCharacteristic>::null(),
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

    pub fn descriptors(&self) -> Vec<Descriptor> {
        let mut descriptors = Vec::<Descriptor>::new();

        for descriptor_wrapper in self.internal.descriptors().iter_mut() {
            descriptors.push(InnerDescriptor::new(descriptor_wrapper).into());
        }

        return descriptors;
    }

    pub fn capabilities(&self) -> Vec<CharacteristicCapability> {
        let mut capabilities = Vec::<CharacteristicCapability>::new();

        if self.internal.can_read() {
            capabilities.push(CharacteristicCapability::Read);
        }

        if self.internal.can_write_request() {
            capabilities.push(CharacteristicCapability::WriteRequest);
        }

        if self.internal.can_write_command() {
            capabilities.push(CharacteristicCapability::WriteCommand);
        }

        if self.internal.can_notify() {
            capabilities.push(CharacteristicCapability::Notify);
        }

        if self.internal.can_indicate() {
            capabilities.push(CharacteristicCapability::Indicate);
        }

        return capabilities;
    }

    pub fn can_read(&self) -> bool {
        return self.internal.can_read();
    }

    pub fn can_write_request(&self) -> bool {
        return self.internal.can_write_request();
    }

    pub fn can_write_command(&self) -> bool {
        return self.internal.can_write_command();
    }

    pub fn can_notify(&self) -> bool {
        return self.internal.can_notify();
    }

    pub fn can_indicate(&self) -> bool {
        return self.internal.can_indicate();
    }
}

unsafe impl Sync for InnerCharacteristic {}
unsafe impl Send for InnerCharacteristic {}

#[derive(Clone)]
pub struct Characteristic {
    inner: Arc<Pin<Box<InnerCharacteristic>>>,
}

impl Characteristic {
    pub fn uuid(&self) -> String {
        return self.inner.uuid();
    }

    pub fn initialized(&self) -> bool {
        return self.inner.initialized();
    }

    pub fn descriptors(&self) -> Vec<Descriptor> {
        return self.inner.descriptors();
    }

    pub fn capabilities(&self) -> Vec<CharacteristicCapability> {
        return self.inner.capabilities();
    }

    pub fn can_read(&self) -> bool {
        return self.inner.can_read();
    }

    pub fn can_write_request(&self) -> bool {
        return self.inner.can_write_request();
    }

    pub fn can_write_command(&self) -> bool {
        return self.inner.can_write_command();
    }

    pub fn can_notify(&self) -> bool {
        return self.inner.can_notify();
    }

    pub fn can_indicate(&self) -> bool {
        return self.inner.can_indicate();
    }
}

impl From<Pin<Box<InnerCharacteristic>>> for Characteristic {
    fn from(characteristic: Pin<Box<InnerCharacteristic>>) -> Self {
        return Characteristic {
            inner: Arc::new(characteristic),
        };
    }
}

unsafe impl Send for Characteristic {}
unsafe impl Sync for Characteristic {}
