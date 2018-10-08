library(brainflow)

board_shim <- get_board_shim(Boards()$Cython["Type"], "/dev/ttyUSB0")
prepare_session(board_shim)
start_stream(board_shim, 3600)
Sys.sleep(time = 5)
stop_stream(board_shim)
data <- get_current_board_data(board_shim, 250)
release_session(board_shim)

data_handler <- get_data_handler(Boards()$Cython["Type"], numpy_data = data)
preprocess_data(data_handler, 1, 50, TRUE)
preprocessed_data <- get_data(data_handler)
head(preprocessed_data)
