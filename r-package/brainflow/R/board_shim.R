#' @export
Boards <- function () {
    list(Cython = 0, Unimplemented = 1)
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
    board_shim$start_stream(max_size)
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
    board_shim$get_current_board_data(num_samples)
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
