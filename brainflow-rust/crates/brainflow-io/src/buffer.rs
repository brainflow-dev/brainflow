//! Ring buffer implementation for data streaming.

use alloc::vec::Vec;
use brainflow_sys::sync::SpinLock;
use core::cell::UnsafeCell;

/// A thread-safe ring buffer for storing samples.
///
/// This matches the BrainFlow DataBuffer implementation.
pub struct RingBuffer<T> {
    data: UnsafeCell<Vec<T>>,
    capacity: usize,
    sample_size: usize,
    head: UnsafeCell<usize>,
    tail: UnsafeCell<usize>,
    count: UnsafeCell<usize>,
    lock: SpinLock,
}

impl<T: Clone + Default> RingBuffer<T> {
    /// Create a new ring buffer.
    ///
    /// # Arguments
    /// * `capacity` - Maximum number of samples to store.
    /// * `sample_size` - Number of elements per sample.
    pub fn new(capacity: usize, sample_size: usize) -> Self {
        let total_size = capacity * sample_size;
        Self {
            data: UnsafeCell::new(alloc::vec![T::default(); total_size]),
            capacity,
            sample_size,
            head: UnsafeCell::new(0),
            tail: UnsafeCell::new(0),
            count: UnsafeCell::new(0),
            lock: SpinLock::new(),
        }
    }

    /// Get the capacity in samples.
    pub fn capacity(&self) -> usize {
        self.capacity
    }

    /// Get the sample size.
    pub fn sample_size(&self) -> usize {
        self.sample_size
    }

    /// Get the number of samples currently stored.
    pub fn len(&self) -> usize {
        self.lock.lock();
        let count = unsafe { *self.count.get() };
        self.lock.unlock();
        count
    }

    /// Check if the buffer is empty.
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }

    /// Check if the buffer is full.
    pub fn is_full(&self) -> bool {
        self.len() >= self.capacity
    }

    /// Add a sample to the buffer.
    ///
    /// If the buffer is full, the oldest sample is overwritten.
    pub fn push(&self, sample: &[T]) {
        if sample.len() != self.sample_size {
            return;
        }

        self.lock.lock();

        unsafe {
            let data = &mut *self.data.get();
            let tail = *self.tail.get();
            let count = &mut *self.count.get();
            let head = &mut *self.head.get();

            // Copy sample data
            let start = tail * self.sample_size;
            for (i, val) in sample.iter().enumerate() {
                data[start + i] = val.clone();
            }

            // Update tail
            *self.tail.get() = (tail + 1) % self.capacity;

            // Update count and head if full
            if *count < self.capacity {
                *count += 1;
            } else {
                *head = (*head + 1) % self.capacity;
            }
        }

        self.lock.unlock();
    }

    /// Get samples from the buffer (removes them).
    ///
    /// Returns up to `max_samples` samples.
    pub fn pop(&self, max_samples: usize) -> Vec<T> {
        self.lock.lock();

        let samples = unsafe {
            let data = &*self.data.get();
            let head = &mut *self.head.get();
            let count = &mut *self.count.get();

            let num_samples = max_samples.min(*count);
            let mut result = Vec::with_capacity(num_samples * self.sample_size);

            for _ in 0..num_samples {
                let start = *head * self.sample_size;
                for i in 0..self.sample_size {
                    result.push(data[start + i].clone());
                }
                *head = (*head + 1) % self.capacity;
                *count -= 1;
            }

            result
        };

        self.lock.unlock();
        samples
    }

    /// Get samples from the buffer without removing them.
    ///
    /// Returns up to `max_samples` of the most recent samples.
    pub fn peek(&self, max_samples: usize) -> Vec<T> {
        self.lock.lock();

        let samples = unsafe {
            let data = &*self.data.get();
            let tail = *self.tail.get();
            let count = *self.count.get();

            let num_samples = max_samples.min(count);
            let mut result = Vec::with_capacity(num_samples * self.sample_size);

            // Start from the oldest sample we want
            let start_idx = if count > num_samples {
                (tail + self.capacity - num_samples) % self.capacity
            } else {
                (tail + self.capacity - count) % self.capacity
            };

            for i in 0..num_samples {
                let idx = (start_idx + i) % self.capacity;
                let start = idx * self.sample_size;
                for j in 0..self.sample_size {
                    result.push(data[start + j].clone());
                }
            }

            result
        };

        self.lock.unlock();
        samples
    }

    /// Clear all data from the buffer.
    pub fn clear(&self) {
        self.lock.lock();
        unsafe {
            *self.head.get() = 0;
            *self.tail.get() = 0;
            *self.count.get() = 0;
        }
        self.lock.unlock();
    }
}

// Safety: RingBuffer uses internal locking for thread safety.
unsafe impl<T: Send> Send for RingBuffer<T> {}
unsafe impl<T: Send> Sync for RingBuffer<T> {}

/// A simple data buffer for double-precision samples.
pub type DataBuffer = RingBuffer<f64>;

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;

    #[test]
    fn test_push_pop() {
        let buffer: RingBuffer<f64> = RingBuffer::new(4, 2);

        buffer.push(&[1.0, 2.0]);
        buffer.push(&[3.0, 4.0]);
        buffer.push(&[5.0, 6.0]);

        assert_eq!(buffer.len(), 3);

        let data = buffer.pop(2);
        assert_eq!(data, vec![1.0, 2.0, 3.0, 4.0]);
        assert_eq!(buffer.len(), 1);
    }

    #[test]
    fn test_overflow() {
        let buffer: RingBuffer<f64> = RingBuffer::new(2, 1);

        buffer.push(&[1.0]);
        buffer.push(&[2.0]);
        buffer.push(&[3.0]); // Overwrites oldest

        assert_eq!(buffer.len(), 2);

        let data = buffer.pop(2);
        assert_eq!(data, vec![2.0, 3.0]);
    }

    #[test]
    fn test_peek() {
        let buffer: RingBuffer<f64> = RingBuffer::new(4, 2);

        buffer.push(&[1.0, 2.0]);
        buffer.push(&[3.0, 4.0]);

        let peeked = buffer.peek(1);
        assert_eq!(peeked, vec![3.0, 4.0]); // Most recent

        assert_eq!(buffer.len(), 2); // Still there
    }
}
