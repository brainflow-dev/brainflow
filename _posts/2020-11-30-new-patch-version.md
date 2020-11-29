---
layout: post
title: BrainFlow 3.7.2
image: /img/github_actions2.png
tags: [release]
---

### Summary of Changes:

* Move all CI pipelines to Github Actions
* Add C# support for Linux and MacOS 
* Add support for Unicorn device on Windows

#### Github Actions

Despite the fact that this change doesn't affect end users directly, it's the most important and biggest update in this release. Also, it affects all BrainFlow developers.

Before we used Travis for Unix like systems and Appveyour for Windows. It was a standard for open source projects for a long time and when I started BrainFlow there was no alternative to these CI systems. Github added Github Actions later and we thought about moving to it, since it has several really cool features, but we also didn't want to rewrite working pipelines and kept as is.

Recently Travis changed their subscription model and in fact it's not free for open source projects anymore. More info about this change can be found [here](https://blog.travis-ci.com/2020-11-02-travis-ci-new-billing). Due to it we could not run MacOS tests and deploy script, so we had to find a new option. So, we moved to Github Actions, it works much faster and better, allows you to reuse code written by others and has other advantages.

Transition to Github Actions was seamless in general and now pipelines are much more modular and look better, but there is a little fly in the ointment. There are no images for MacOS 10.13 or 10.14, it's still a problem, because it determines the lowest MacOS version which we support in BrainFlow. To don't drop support for MacOS 10.13 and 10.14 we configured self-hosted runner to build release libraries. To don't keep self-hosted runner up and running all the time, for testing PRs we will use runners provided by Github Actions with 10.15 and turn on self-hosted runner only to build releases.

In a long term support for old MacOS versions will be removed, but we will try to make it as smooth as possible and affect as less users as possible.

#### C# support for Unix like systems

Thanks to [GrahamBriggs](https://github.com/GrahamBriggs) for implementing it. Now you can use C# binding on Linux and MacOS using [Mono](https://www.monodevelop.com/).

#### Unicorn on Windows

Thanks to [g.tec](https://www.gtec.at/) for providing Windows library to work with Unicorn. 
