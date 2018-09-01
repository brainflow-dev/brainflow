Sys.setenv(RETICULATE_PYTHON = "/home/andrey/venv_brainflow/bin/python")
library(reticulate)
library(brainflow)

board_shim <- get_board_shim(Boards()$Cython, "/dev/emulated_cython")
prepare_session(board_shim)
start_stream(board_shim, as.integer(3600))
Sys.sleep(time = 5)
stop_stream(board_shim)
data <- get_board_data(board_shim)

data_handler <- get_data_handler(Boards()$Cython, data)
preprocess_data(data_handler, 1, 50, FALSE)

release_session(board_shim)
