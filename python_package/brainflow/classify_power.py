
!pip install --upgrade brainflow scikit-learn

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter
import numpy as np
from sklearn.svm import SVC
from scipy import signal

def classify_power_features(data, sampling_rate, bands, labels=None):
    """Classify brain state from band powers using SVM. Single chunk version."""
    if len(data.shape) == 1:
        data = data.reshape(1, -1)
    n_samples = data.shape[1]
    
    # Inspect data
    print(f"EEG data shape: {data[0].shape}")
    print(f"Sample EEG data: {data[0][:10]}")
    if np.any(np.isnan(data[0])) or np.any(np.isinf(data[0])):
        print("Warning: EEG data contains NaNs or Infs")
        return "Error: Invalid data"

    # Extract powers for channel 1
    powers = []
    for low, high in bands:
        # SciPy bandpass filter
        nyquist = 0.5 * sampling_rate
        low_norm = low / nyquist
        high_norm = high / nyquist
        b, a = signal.butter(2, [low_norm, high_norm], btype='band')
        filtered_data = signal.lfilter(b, a, data[0])
        
        power = np.mean(np.abs(filtered_data)**2) #power calculation
        powers.append(power)
    
    # Features: [alpha_power, beta_power]
    features = np.array(powers).reshape(1, -1)
    
    # Hardcoded SVM (trained offline for demo)
    X_train = np.array([[0.8, 0.2], [0.3, 0.6]])  # Alpha, beta powers
    y_train = np.array([0, 1])  # 0=rest, 1=focus
    clf = SVC(kernel='rbf')
    clf.fit(X_train, y_train)
    
    # Predict
    pred = clf.predict(features)[0]
    return "rest" if pred == 0 else "focus"

# Test with synthetic data
BoardShim.enable_dev_board_logger()
params = BrainFlowInputParams()
board = BoardShim(BoardIds.SYNTHETIC_BOARD.value, params)
board.prepare_session()
board.start_stream()
import time
time.sleep(5)
data = board.get_board_data()
board.stop_stream()
board.release_session()

eeg_data = data[1]  # Channel 1
fs = 250
bands = [[8, 13], [13, 30]]
state = classify_power_features(eeg_data, fs, bands)
print(f"Predicted State: {state}")
