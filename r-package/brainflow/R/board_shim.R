#' @export
BoardIds <- function () {
    list (
        STREAMING_BOARD = c (Id = as.integer (-2)),
        SYNTHETIC_BOARD = c (Id = as.integer (-1)),
        CYTON_BOARD = c (Id = as.integer (0)),
        GANGLION_BOARD = c (Id = as.integer (1)),
        CYTON_DAISY_BOARD = c (Id = as.integer (2)),
        NOVAXR_BOARD = c (Id = as.integer (3)),
        GANGLION_WIFI_BOARD = c (Id = as.integer (4)),
        CYTON_WIFI_BOARD = c (Id = as.integer (5)),
        CYTON_DAISY_WIFI_BOARD = c (Id = as.integer (6))
    )
}

#' @param board_id
#' @param input_params
#' @return BoardShim object
#' @export
get_board_shim_object <- function (board_id, input_params)
{
    brainflow$BoardShim (board_id, input_params)
}

#' @return BoardShim class
#' @export
get_board_shim_class <- function ()
{
    brainflow$BoardShim
}

#' @return BrainFlowInputParams class
#' @export
get_brainflow_input_params <- function ()
{
    brainflow$BrainFlowInputParams ()
}