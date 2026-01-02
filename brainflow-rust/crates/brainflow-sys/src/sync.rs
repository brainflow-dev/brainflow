//! Synchronization primitives.

use core::cell::UnsafeCell;
use core::sync::atomic::{AtomicBool, AtomicUsize, Ordering};

/// A simple spinlock for low-contention scenarios.
///
/// This matches the C++ SpinLock implementation used in BrainFlow.
pub struct SpinLock {
    locked: AtomicBool,
}

impl SpinLock {
    /// Create a new unlocked spinlock.
    #[must_use]
    pub const fn new() -> Self {
        Self {
            locked: AtomicBool::new(false),
        }
    }

    /// Acquire the lock, spinning until available.
    #[inline]
    pub fn lock(&self) {
        while self
            .locked
            .compare_exchange_weak(false, true, Ordering::Acquire, Ordering::Relaxed)
            .is_err()
        {
            // Spin with a hint to reduce power consumption
            core::hint::spin_loop();
        }
    }

    /// Release the lock.
    #[inline]
    pub fn unlock(&self) {
        self.locked.store(false, Ordering::Release);
    }

    /// Try to acquire the lock without blocking.
    #[inline]
    pub fn try_lock(&self) -> bool {
        self.locked
            .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
            .is_ok()
    }
}

impl Default for SpinLock {
    fn default() -> Self {
        Self::new()
    }
}

// Safety: SpinLock uses atomic operations and is safe to share between threads.
unsafe impl Send for SpinLock {}
unsafe impl Sync for SpinLock {}

/// A ticket lock for fair ordering (FIFO).
///
/// This matches the C++ TicketLock implementation.
pub struct TicketLock {
    next_ticket: AtomicUsize,
    serving: AtomicUsize,
}

impl TicketLock {
    /// Create a new ticket lock.
    #[must_use]
    pub const fn new() -> Self {
        Self {
            next_ticket: AtomicUsize::new(0),
            serving: AtomicUsize::new(0),
        }
    }

    /// Acquire the lock with fair ordering.
    #[inline]
    pub fn lock(&self) {
        let ticket = self.next_ticket.fetch_add(1, Ordering::Relaxed);
        while self.serving.load(Ordering::Acquire) != ticket {
            core::hint::spin_loop();
        }
    }

    /// Release the lock.
    #[inline]
    pub fn unlock(&self) {
        self.serving.fetch_add(1, Ordering::Release);
    }
}

impl Default for TicketLock {
    fn default() -> Self {
        Self::new()
    }
}

// Safety: TicketLock uses atomic operations and is safe to share between threads.
unsafe impl Send for TicketLock {}
unsafe impl Sync for TicketLock {}

/// A mutex-like guard for spinlock.
pub struct SpinLockGuard<'a, T> {
    lock: &'a SpinLock,
    data: &'a mut T,
}

impl<T> core::ops::Deref for SpinLockGuard<'_, T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        self.data
    }
}

impl<T> core::ops::DerefMut for SpinLockGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        self.data
    }
}

impl<T> Drop for SpinLockGuard<'_, T> {
    fn drop(&mut self) {
        self.lock.unlock();
    }
}

/// A spinlock-protected value.
pub struct Mutex<T> {
    lock: SpinLock,
    data: UnsafeCell<T>,
}

impl<T> Mutex<T> {
    /// Create a new mutex with the given value.
    pub const fn new(data: T) -> Self {
        Self {
            lock: SpinLock::new(),
            data: UnsafeCell::new(data),
        }
    }

    /// Lock the mutex and return a guard.
    pub fn lock(&self) -> SpinLockGuard<'_, T> {
        self.lock.lock();
        SpinLockGuard {
            lock: &self.lock,
            // Safety: We hold the lock, so exclusive access is guaranteed.
            data: unsafe { &mut *self.data.get() },
        }
    }

    /// Try to lock the mutex without blocking.
    pub fn try_lock(&self) -> Option<SpinLockGuard<'_, T>> {
        if self.lock.try_lock() {
            Some(SpinLockGuard {
                lock: &self.lock,
                // Safety: We hold the lock, so exclusive access is guaranteed.
                data: unsafe { &mut *self.data.get() },
            })
        } else {
            None
        }
    }
}

// Safety: Mutex protects access to T with a spinlock.
unsafe impl<T: Send> Send for Mutex<T> {}
unsafe impl<T: Send> Sync for Mutex<T> {}
