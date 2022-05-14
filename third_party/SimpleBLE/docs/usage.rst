Usage
=====

Please visit our examples page for more detailed instructions.

Concurrency
~~~~~~~~~~~

When designing your application using SimpleBLE, concurrency is a key
aspect that needs to be taken into account. This is because internally
the library relies on a thread pool to handle asynchronous operations
and poll the OS’s Bluetooth stack, which can suffer from contention and
potentially cause the program to crash or freeze if these threads are
significantly delayed.

This can have an important effect when using SimpleBLE with UI
applications, such as WxWidgets or Unity.

A future version of the library will rely on the use of a thread pool to
run callbacks, thus mitigating the potential of these problems.