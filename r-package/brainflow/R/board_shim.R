#' @export
Boards <- function () {
    list(Cyton = c(Type = as.integer(0), sampling = as.integer(250), num_channels = as.integer(13), num_eeg_channels = as.integer(8)),
         Unimplemented = c(Type = 1, sampling = NULL, num_channels = NULL, num_eeg_channels = NULL))
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
