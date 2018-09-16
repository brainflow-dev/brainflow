# R
This package works via [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html) which allows to call Python function directly from R, also it translates all Python classes(even user defined) to the corresponds R constructures 
For now it is not added to CRAN so you will have to build this package manually using R-strudio

## Example:
```
library(reticulate)
use_python("/home/osboxes/venv_brainflow/bin/python")
library(brainflow)

board_shim <- get_board_shim(Boards()$Cython["Type"], "/dev/emulated_cython")
prepare_session(board_shim)
start_stream(board_shim, 3600)
Sys.sleep(time = 5)
stop_stream(board_shim)
data <- get_current_board_data(board_shim, 250)

data_handler <- get_data_handler(Boards()$Cython["Type"], data)
preprocess_data(data_handler, 1, 50, TRUE)
preprocessed_data <- get_data(data_handler)
head(preprocessed_data)
release_session(board_shim)
```
All methods provided by this package can be found [here](https://github.com/Andrey1994/brainflow/tree/master/r-package/brainflow/R).

Also [GUI](https://github.com/Andrey1994/brainflow/tree/master/gui) is implemented using brainflow R package and Shiny
