% Two simultaneous mixtures: rows are channels and columns are samples.
time = (0:1023) / 256;
first = sin(2 * pi * 7 * time);
second = sin(2 * pi * 13 * time).^3;
data = [first + 0.3 * second; 0.2 * first + second];
[w, k, a, s] = DataFilter.perform_ica(data, 2);
% Component order and sign are arbitrary.
fprintf('Recovered %d sources from %d samples\n', size(s, 1), size(s, 2));
