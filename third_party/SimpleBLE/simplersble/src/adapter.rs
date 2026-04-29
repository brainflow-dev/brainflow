use std::pin::Pin;
use std::mem;
use std::sync::{Arc, Mutex};

use tokio::sync::broadcast;
use tokio_stream::wrappers::BroadcastStream;
use futures::{Stream, TryStreamExt};

use super::ffi;
use crate::peripheral::InnerPeripheral;
use crate::peripheral::Peripheral;
use crate::types::Error;

#[derive(Clone)]
pub enum ScanEvent {
    Start,
    Stop,
    Found(Peripheral),
    Updated(Peripheral),
}

#[derive(Clone)]
pub enum PowerEvent {
    On,
    Off,
}

pub struct InnerAdapter {
    internal: cxx::UniquePtr<ffi::RustyAdapter>,
    on_scan_event: broadcast::Sender<ScanEvent>,
    on_power_event: broadcast::Sender<PowerEvent>,
}

impl InnerAdapter {
    pub fn bluetooth_enabled() -> Result<bool, Error> {
        ffi::RustyAdapter_bluetooth_enabled().map_err(Error::from_cxx_exception)
    }

    pub fn get_adapters() -> Result<Vec<Pin<Box<InnerAdapter>>>, Error> {
        let mut raw_adapter_list = ffi::RustyAdapter_get_adapters().map_err(Error::from_cxx_exception)?;

        let mut adapters = Vec::<Pin<Box<InnerAdapter>>>::new();
        for adapter_wrapper in raw_adapter_list.iter_mut() {
            adapters.push(InnerAdapter::new(adapter_wrapper));
        }
        Ok(adapters)
    }

    fn new(wrapper: &mut ffi::RustyAdapterWrapper) -> Pin<Box<Self>> {
        let (scan_event_sender, _) = broadcast::channel(128);
        let (power_event_sender, _) = broadcast::channel(128);

        let this = Self {
            internal: cxx::UniquePtr::<ffi::RustyAdapter>::null(),
            on_scan_event: scan_event_sender,
            on_power_event: power_event_sender,
        };

        let mut this_boxed = Box::pin(this);
        wrapper.internal.link(this_boxed.as_mut()).unwrap();
        mem::swap(&mut this_boxed.internal, &mut wrapper.internal);

        return this_boxed;
    }

    pub fn initialized(&self) -> Result<bool, Error> {
        self.internal.initialized().map_err(Error::from_cxx_exception)
    }

    pub fn identifier(&self) -> Result<String, Error> {
        self.internal.identifier().map_err(Error::from_cxx_exception)
    }

    pub fn address(&self) -> Result<String, Error> {
        self.internal.address().map_err(Error::from_cxx_exception)
    }

    pub fn power_on(&self) -> Result<(), Error> {
        self.internal.power_on().map_err(Error::from_cxx_exception)
    }

    pub fn power_off(&self) -> Result<(), Error> {
        self.internal.power_off().map_err(Error::from_cxx_exception)
    }

    pub fn is_powered(&self) -> Result<bool, Error> {
        self.internal.is_powered().map_err(Error::from_cxx_exception)
    }

    pub fn scan_start(&self) -> Result<(), Error> {
        self.internal.scan_start().map_err(Error::from_cxx_exception)
    }

    pub fn scan_stop(&self) -> Result<(), Error> {
        self.internal.scan_stop().map_err(Error::from_cxx_exception)
    }

    pub fn scan_for(&self, timeout_ms: i32) -> Result<(), Error> {
        self.internal.scan_for(timeout_ms).map_err(Error::from_cxx_exception)
    }

    pub fn scan_is_active(&self) -> Result<bool, Error> {
        self.internal.scan_is_active().map_err(Error::from_cxx_exception)
    }

    pub fn scan_get_results(&self) -> Result<Vec<Peripheral>, Error> {
        let mut raw_peripheral_list = self.internal.scan_get_results().map_err(Error::from_cxx_exception)?;

        let mut peripherals = Vec::<Peripheral>::new();
        for peripheral_wrapper in raw_peripheral_list.iter_mut() {
            peripherals.push(InnerPeripheral::new(peripheral_wrapper).into());
        }

        return Ok(peripherals);
    }

    pub fn get_paired_peripherals(&self) -> Result<Vec<Peripheral>, Error> {
        let mut raw_peripheral_list =
            self.internal.get_paired_peripherals().map_err(Error::from_cxx_exception)?;

        let mut peripherals = Vec::<Peripheral>::new();
        for peripheral_wrapper in raw_peripheral_list.iter_mut() {
            peripherals.push(InnerPeripheral::new(peripheral_wrapper).into());
        }

        return Ok(peripherals);
    }

    pub fn get_connected_peripherals(&self) -> Result<Vec<Peripheral>, Error> {
        let mut raw_peripheral_list =
            self.internal.get_connected_peripherals().map_err(Error::from_cxx_exception)?;

        let mut peripherals = Vec::<Peripheral>::new();
        for peripheral_wrapper in raw_peripheral_list.iter_mut() {
            peripherals.push(InnerPeripheral::new(peripheral_wrapper).into());
        }

        return Ok(peripherals);
    }

    pub fn on_callback_scan_start(&self) {
        // TODO: Review how to handle errors here.
        let _ = self.on_scan_event.send(ScanEvent::Start);
    }

    pub fn on_callback_scan_stop(&self) {
        // TODO: Review how to handle errors here.
        let _ = self.on_scan_event.send(ScanEvent::Stop);
    }

    pub fn on_callback_scan_updated(&self, peripheral: &mut ffi::RustyPeripheralWrapper) {
        // TODO: Review how to handle errors here.
        let peripheral: Peripheral = InnerPeripheral::new(peripheral).into();
        let _ = self.on_scan_event.send(ScanEvent::Updated(peripheral));
    }

    pub fn on_callback_scan_found(&self, peripheral: &mut ffi::RustyPeripheralWrapper) {
        // TODO: Review how to handle errors here.
        let peripheral: Peripheral = InnerPeripheral::new(peripheral).into();
        let _ = self.on_scan_event.send(ScanEvent::Found(peripheral));
    }

    pub fn on_callback_power_on(&self) {
        // TODO: Review how to handle errors here.
        let _ = self.on_power_event.send(PowerEvent::On);
    }

    pub fn on_callback_power_off(&self) {
        // TODO: Review how to handle errors here.
        let _ = self.on_power_event.send(PowerEvent::Off);
    }
}

impl Drop for InnerAdapter {
    fn drop(&mut self) {
        self.internal.unlink().unwrap();
    }
}

unsafe impl Sync for InnerAdapter {}
unsafe impl Send for InnerAdapter {}

#[derive(Clone)]
pub struct Adapter {
    inner: Arc<Mutex<Pin<Box<InnerAdapter>>>>,
}

impl Adapter {

    pub fn bluetooth_enabled() -> Result<bool, Error> {
        ffi::RustyAdapter_bluetooth_enabled().map_err(Error::from_cxx_exception)
    }

    pub fn get_adapters() -> Result<Vec<Adapter>, Error> {
        let mut raw_adapter_list = ffi::RustyAdapter_get_adapters().map_err(Error::from_cxx_exception)?;

        let mut adapters = Vec::<Adapter>::new();
        for adapter_wrapper in raw_adapter_list.iter_mut() {
            adapters.push(InnerAdapter::new(adapter_wrapper).into());
        }

        return Ok(adapters);
    }

    pub fn initialized(&self) -> Result<bool, Error> {
        self.inner.lock().unwrap().initialized()
    }

    pub fn identifier(&self) -> Result<String, Error> {
        self.inner.lock().unwrap().identifier()
    }

    pub fn address(&self) -> Result<String, Error> {
        self.inner.lock().unwrap().address()
    }

    pub fn power_on(&self) -> Result<(), Error> {
        self.inner.lock().unwrap().power_on()
    }

    pub fn power_off(&self) -> Result<(), Error> {
        self.inner.lock().unwrap().power_off()
    }

    pub fn is_powered(&self) -> Result<bool, Error> {
        self.inner.lock().unwrap().is_powered()
    }

    pub fn scan_start(&self) -> Result<(), Error> {
        self.inner.lock().unwrap().scan_start()
    }

    pub fn scan_stop(&self) -> Result<(), Error> {
        self.inner.lock().unwrap().scan_stop()
    }

    pub fn scan_for(&self, timeout_ms: i32) -> Result<(), Error> {
        self.inner.lock().unwrap().scan_for(timeout_ms)
    }

    pub fn scan_is_active(&self) -> Result<bool, Error> {
        self.inner.lock().unwrap().scan_is_active()
    }

    pub fn scan_get_results(&self) -> Result<Vec<Peripheral>, Error> {
        self.inner.lock().unwrap().scan_get_results()
    }

    pub fn get_paired_peripherals(&self) -> Result<Vec<Peripheral>, Error> {
        self.inner.lock().unwrap().get_paired_peripherals()
    }

    pub fn get_connected_peripherals(&self) -> Result<Vec<Peripheral>, Error> {
        self.inner.lock().unwrap().get_connected_peripherals()
    }

    pub fn on_scan_event(&self) -> impl Stream<Item = Result<ScanEvent, Error>> {
        BroadcastStream::new(self.inner.lock().unwrap().on_scan_event.subscribe())
            .map_err(|e| Error::from_string(e.to_string()))
    }

    pub fn on_power_event(&self) -> impl Stream<Item = Result<PowerEvent, Error>> {
        BroadcastStream::new(self.inner.lock().unwrap().on_power_event.subscribe())
            .map_err(|e| Error::from_string(e.to_string()))
    }
}


impl From<Pin<Box<InnerAdapter>>> for Adapter {
    fn from(adapter: Pin<Box<InnerAdapter>>) -> Self {
        Self {
            inner: Arc::new(Mutex::new(adapter)),
        }
    }
}

unsafe impl Send for Adapter {}
unsafe impl Sync for Adapter {}
