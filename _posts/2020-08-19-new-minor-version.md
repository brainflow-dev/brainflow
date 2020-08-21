---
layout: post
title: BrainFlow 3.4.0
image: /img/ml.jpeg
tags: [release]
---

### ML Module

In this version we've added and API to calculate derivative metrics from raw EEG data.

Example:

```python
eeg_channels = BoardShim.get_eeg_channels (args.board_id)
bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, True)
feature_vector = np.concatenate ((bands[0], bands[1]))
print(feature_vector)

# calc concentration
concentration_params = BrainFlowModelParams (BrainFlowMetrics.CONCENTRATION.value, BrainFlowClassifiers.REGRESSION.value)
concentration = MLModel (concentration_params)
concentration.prepare ()
print ('Concentration: %f' % concentration.predict (feature_vector))
concentration.release ()

# calc relaxation
relaxation_params = BrainFlowModelParams (BrainFlowMetrics.RELAXATION.value, BrainFlowClassifiers.REGRESSION.value)
relaxation = MLModel (relaxation_params)
relaxation.prepare ()
print ('Relaxation: %f' % relaxation.predict (feature_vector))
relaxation.release ()
```

Currently it supports only Logistic Regression Classifier, supported metrics are Concentration and Relaxation.

In upcomming versions we will add more classifiers(SVM, KNN) and add an API to load user defined models in ONNX format. Also, dataset we use will be extended, so implemented classifiers will work better.

For implementation details refer to:

* [get_avg_band_powers](https://github.com/brainflow-dev/brainflow/blob/ml/src/data_handler/data_handler.cpp)
* [prepare data and train model](https://github.com/brainflow-dev/brainflow/blob/ml/src/ml/train/focus_classifiers.py)
* [final equation](https://github.com/brainflow-dev/brainflow/blob/master/src/ml/concentration_regression_classifier.cpp)

### Other Changes

Bug fixes:

* [Band Power Calculation](https://github.com/brainflow-dev/brainflow/issues/82)

New Methods:

* Drop *get_log_psd* and *get_log_psd_welch*
* Add *get_avg_band_powers*
* Add *get_exg_channels*

For developers:

* In this update OpenMP was added, we can now easily parallel computations, check *get_avg_band_powers* for reference
