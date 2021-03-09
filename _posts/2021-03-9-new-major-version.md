---
layout: post
title: BrainFlow 4.0.0
subtitle: New BrainFlow Version Released
image: /img/rowmajor.jpg
tags: [release]
author: andrey_parfenov
---



### Breaking change for C++ and C# bindings

* All methods in C++ binding return C++ objects like `std::vector` instead `int*` and `std::string` instead `char*`
* In C++ binding data is represented by `BrainFlowArray<double, 2>` instead `double **`
* Remove Accord as a dependency from C# binding 
* Add `get_csp` method

#### C# binding

Before we used Accord as a dependency and used to return Accord objects to the user. Unfortunately Accord is not supported anymore. It was a great project, but its author deprecated it. You can read more about this announcement [here](https://github.com/accord-net/framework#archiving). We decided to remove unmaintained dependency, and it is a breaking change. We implemented a subset of Accord functions to work with matrices, but not all of them and they are in another namespace.

#### C++ binding

We used to return plain C pointers for almost everything, in this version we moved to C++ objects instead. From now on methods like `get_eeg_channels` return `std::vector`, methods like `get_device_name` return `std::string`.

The most important change here is a `BrainFlowArray` class. It is inspired by Numpy and stores NDArray as a continuous memory chunk in row major order. Before this change we used `double **`, it was much harder to use and prevented us to implement some performance optimizations.

<div style="text-align: center">
    <a href="https://opencollective.com/brainflow" title="brainflow" target="_blank" align="left">
        <img width="1024" height="320" src="https://live.staticflickr.com/65535/51020135638_a68706b32a_b.jpg">
    </a>
</div>

Feel free to check [Docs Page](https://brainflow.readthedocs.io/en/stable/) to learn more about API changes and updated samples.
