#' @param board_id
#' @param eeg_data
#' @return DataHandler object
#' @export
get_data_handler <- function(board_id, eeg_data) {
  brainflow$DataHandler(board_id, eeg_data)
}

#' @param data_handler
#'
#' @export
remove_dc_offset <- function(data_handler) {
  data_handler$remove_dc_offset()
}

#' @param data_handler
#'
#' @export
notch_interference <- function(data_handler) {
  data_handler$notch_interference()
}

#' @param data_handler
#' @param start_fq
#' @param stop_fq
#'
#' @export
bandpass <- function(data_handler, start_fq, stop_fq) {
    data_handler$bandpass(start_fq, stop_fq)
}

#' @param data_handler
#'
#' @export
calculate_fft <- function(data_handler) {
    data_handler$calculate_fft()
}

#' @param data_handler
#' @return  data object
#' @export
get_data <- function(data_handler) {
    data_handler$get_data()
}

#' @param data_handler
#' @param start_fq
#' @param stop_fq
#' @param calc_fft
#'
#' @export
preprocess_data <- function(data_handler, start_fq, stop_fq, calc_fft) {
    data_handler$preprocess_data(start_fq, stop_fq, calc_fft)
}
