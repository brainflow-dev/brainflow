BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

% In this toy example, think of data as two matrices (epochs), rows - times, columns - channels
data = [6 3 1 5; 3 0 5 1];
data(:,:,2) = [1 5 6 2; 5 1 2 2];

% Labels mean classes corresponding to each epoch
labels = [0 1];

filters_true = [-0.313406 0.0792153; -0.280803  -0.480046];
eigenvalues_true = [0.45671339 0.75297906];

[filters, eigenvalues] = DataFilter.get_csp(data, labels);

disp("evaluated filters = ");
disp(filters);
disp("true filters = ");
disp(filters_true);

disp("evaluated eigenvalues = ");
disp(filters);
disp("true eigenvalues = ");
disp(filters_true);