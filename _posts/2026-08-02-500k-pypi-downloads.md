---
layout: post
title: BrainFlow Passes 500,000 Downloads on PyPI
subtitle: A milestone for BrainFlow's Python package and the community building with it
tags: [history, python]
image: /img/500k-pypi-downloads.png
author: andrey_parfenov
---

BrainFlow's Python package has passed **500,000 downloads on PyPI**. At the time
of writing, [PePy reports 552,303 all-time downloads](https://pepy.tech/projects/brainflow),
including 41,938 during the previous 30 days.

<figure style="margin: 2rem 0; text-align: center;">
  <img src="{{ '/img/500k-pypi-downloads.png' | relative_url }}"
       alt="500K milestone surrounded by biosignal and circuit motifs"
       style="max-width: 100%; height: auto;">
</figure>

Python has long been BrainFlow's most widely used binding. The package provides
one API for acquiring data from supported biosensors, processing signals, and
running machine-learning models, while fitting naturally into Python research,
notebook, prototyping, and production workflows.

```bash
pip install brainflow
```

<figure style="margin: 2rem 0; text-align: center;">
  <a href="https://pepy.tech/projects/brainflow" target="_blank" rel="noopener noreferrer">
    <img src="{{ '/img/brainflow-pepy-downloads.png' | relative_url }}"
         alt="PePy statistics for BrainFlow showing more than 552,000 all-time PyPI downloads"
         style="max-width: 100%; height: auto; border: 1px solid #e1e1e1;">
  </a>
  <figcaption style="margin-top: 0.75rem; color: #666; font-size: 0.9em;">
    PePy snapshot captured August 2, 2026. The displayed statistics include CI traffic.
  </figcaption>
</figure>

A package download is not the same as a unique user. Download counts can include
upgrades, repeated installs, new environments, and automated CI jobs; PePy marks
the view above as including CI traffic. We see this as a milestone for the reach
of the package, not a claim that BrainFlow has 500,000 individual users.

The number still reflects a remarkable amount of activity around open biosensor
software. BrainFlow is used for experiments, teaching, prototypes, applications,
and data pipelines across many boards and operating systems. Every bug report,
device contribution, documentation improvement, and shared example has helped
make that possible.

Thank you to everyone who has installed BrainFlow, contributed code, tested new
hardware, opened an issue, or helped another user in the community.

You can explore the [Python package on PyPI](https://pypi.org/project/brainflow/),
read the [BrainFlow documentation](https://brainflow.readthedocs.io/en/stable/),
try the [Python examples](https://github.com/brainflow-dev/brainflow/tree/master/python_package/examples),
or [contribute on GitHub](https://github.com/brainflow-dev/brainflow).
