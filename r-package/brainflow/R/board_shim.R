#' @export
Boards <- function () {
    list(Cyton = c(Type = as.integer(0), sampling = as.integer(250), num_channels = as.integer(13), num_eeg_channels = as.integer(8)),
         Ganglion = c(Type = as.integer(1), sampling = as.integer(200), num_channels = as.integer(9), num_eeg_channels = as.integer(4)),
         Synthetic = c(Type = as.integer(-1), sampling = as.integer(256), num_channels = as.integer(13), num_eeg_channels = as.integer(8)),
         CytonDaisy = c(Type = as.integer(2), sampling = as.integer(125), num_channels = as.integer(21), num_eeg_channels = as.integer(16)),
         Novaxr = c(Type = as.integer(3), sampling = as.integer(2000), num_channels = as.integer(26), num_eeg_channels = as.integer(16)),
         CytonWifi = c(Type = as.integer(5), sampling = as.integer(1000), num_channels = as.integer(13), num_eeg_channels = as.integer(8)),
         CytonDaisyWifi = c(Type = as.integer(6), sampling = as.integer(1000), num_channels = as.integer(21), num_eeg_channels = as.integer(16)),
         GanglionWifi = c(Type = as.integer(4), sampling = as.integer(1600), num_channels = as.integer(9), num_eeg_channels = as.integer(4)))
}

#' @param board_id
#' @param port_name
#' @return BoardShim object
#' @export
get_board_shim <- function(board_id, port_name) {
    brainflow$BoardShim(board_id, port_name)
}

#' @param board_shim
#'
#' @export
prepare_session <- function(board_shim) {
    board_shim$prepare_session()
}

#' @param board_shim
#' @param max_size
#'
#' @export
start_stream <- function(board_shim, max_size) {
    board_shim$start_stream(as.integer(max_size))
}

#' @param board_shim
#'
#' @export
stop_stream <- function(board_shim) {
    board_shim$stop_stream()
}

#' @param board_shim
#'
#' @export
release_session <- function(board_shim) {
    board_shim$release_session()
}

#' @param board_shim
#' @param num_samples
#' @return current board data
#' @export
get_current_board_data <- function(board_shim, num_samples) {
    board_shim$get_current_board_data(as.integer(num_samples))
}

#' @param board_shim
#' @return immediate board data
#' @export
get_immediate_board_data <- function(board_shim) {
    board_shim$get_immediate_board_data()
}

#' @param board_shim
#' @return board data count
#' @export
get_board_data_count <- function(board_shim) {
    board_shim$get_board_data_count()
}

#' @param board_shim
#' @return board data
#' @export
get_board_data <- function(board_shim) {
    board_shim$get_board_data()
}

#' @param board_shim
#' @param config
#'
#' @export
config_board <- function(board_shim, config) {
    board_shim$config_board(config)
}

#' @param board_shim
#' @param log_file
#'
#' @export
set_log_file <- function(board_shim, log_file) {
    board_shim$set_log_file(log_file)
}