% Fifteen seconds of shared red/IR pulsatility, including filter settling margins.
sampling_rate = int32(128);
time = (0:(15 * double(sampling_rate) - 1)) / double(sampling_rate);
pulse = sin(2 * pi * 1.2 * time) + 0.15 * sin(2 * pi * 2.4 * time);
ir_data = 100000 + 1000 * pulse;
red_data = 80000 + 400 * pulse;

% Example calibration coefficients; real sensors require their own calibration.
spo2 = DataFilter.get_oxygen_level(ir_data, red_data, sampling_rate, 0.0, -37.663, 114.91);
heart_rate = DataFilter.get_heart_rate(ir_data, red_data, sampling_rate, 1024);
fprintf('Synthetic SpO2: %.2f; heart rate: %.2f bpm\n', spo2, heart_rate);
