---
layout: post
title: BrainFlow 3.4.1
image: /img/update.jpg
tags: [release]
---

Changes in this release:

* Add KNN classifier for Concentration and Relaxation, it provides better accuracy and recall than Logistic Regression. You can override number of neighbors via *other_info* field in *BrainFlowModelParams*
* Fix [OpenMP linking issue](https://github.com/brainflow-dev/brainflow/issues/86)
