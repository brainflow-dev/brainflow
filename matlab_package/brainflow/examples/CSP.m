BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

n_epochs = 2;
n_channels = 2;
n_times = 4;

% In this toy example mathematically we have two matrices (epochs) with rows as times and columns as channels
% Here data is a [n_channels, n_times, n_epochs] array
data = [6 3 1 5; 3 0 5 1];
data(:,:,2) = [1 5 6 2; 5 1 2 2];

% Re-order dimensions so that data array has dimensions [n_epochs, n_channels,
% n_times] as it's required by get_csp method
data = permute(data,[3 1 2]);

% Labels mean classes corresponding to each epoch
labels = [0 1];

% True values to compare with
filters_true = [-0.313406 0.0792153; -0.280803  -0.480046];
eigenvalues_true = [0.45671339 0.75297906];

% Calc filters and eigenvalues and compare them
[filters, eigenvalues] = DataFilter.get_csp(data, labels);
