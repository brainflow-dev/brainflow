# R
This package works via [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html) which allows to call Python function directly from R, also it translates all Python classes(even user defined) to the corresponds R constructures.

For now it is not added to the CRAN, so you will have to build this package manually using R-strudio

## Example:
```

library(brainflow)

board_shim <- get_board_shim(Boards()$Cyton["Type"], "/dev/ttyUSB0")
prepare_session(board_shim)
start_stream(board_shim, 3600)
Sys.sleep(time = 5)
stop_stream(board_shim)
data <- get_current_board_data(board_shim, 250)
release_session(board_shim)

data_handler <- get_data_handler(Boards()$Cyton["Type"], numpy_data = data)
preprocess_data(data_handler, 3, 1, 50, TRUE)
preprocessed_data <- get_data(data_handler)
head(preprocessed_data)
save_csv(data_handler, "results.csv")
head(get_data(get_data_handler(Boards()$Cyton["Type"], csv_file = "results.csv")))
```
All methods provided by this package can be found [here](https://github.com/Andrey1994/brainflow/tree/master/r-package/brainflow/R).

Also [GUI](https://github.com/Andrey1994/brainflow/tree/master/gui) is implemented using brainflow R package and Shiny
