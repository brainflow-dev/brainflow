---
layout: post
title: Roadmap for Rust
image: /img/rust.jpg
author: andrey_parfenov
---

## Potential benefits of using Rust

There are several reasons why we would like to add Rust to our codebase:

* There are IO libraries which don't have good alternatives in C/C++, especially for [BLE](https://github.com/deviceplug/btleplug)
* Rust solves memory management problems, it should make our codebase safer
* It should simplify process of adding new boards for software engineers who don't know C/C++

## Rust and C++ interoperability

We are experimenting with Rust. For the foreseeable future, C++ is the reigning monarch in our codebase, and any use of Rust will need to fit in with C++ — not the other way around. This seems to present some C++/Rust interoperability challenges which nobody else has faced.

C/C++ should be the main language and all bindings should load C/C++ dynamic libraries and should not interact with Rust directly.

There are two options which we are considering for C++/Rust interoperability:

* [FFI](https://doc.rust-lang.org/nomicon/ffi.html)
* Compile Rust code to dynamic library and use smth like `DynLibBoard` in C/C++

We don't like any solutions which involve Inter Process Communication, we force the idea of direct communication with devices and not going to make any compromises here.

## Our Plan

We think the hardest part of this is imagining a safe way to pass types and all required info between Rust and C++. Also, we need to avoid duplication and don't want to rewrite existing code from C/C++.

Roadmap:

* First of all we want to develop Rust binding for the users. There were just a few requests for that, but it should be a good starting point for us to learn Rust better [#311](https://github.com/brainflow-dev/brainflow/issues/311)
* Develop a bridge between C++ and Rust code [#312](https://github.com/brainflow-dev/brainflow/issues/312) 
* Use Rust BLE library for new devices [#313](https://github.com/brainflow-dev/brainflow/issues/313)
* Optional: if everything works well we can consider moving currently supported devices(especially BLE based) from C++ to Rust
