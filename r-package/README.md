# R
This package works via [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html) which allows to call Python function directly from R, also it translates all Python classes(even user defined) to the corresponds R constructures.

For now it is not added to the CRAN, so you will have to build this package manually using R-strudio or from command line and install python brainflow bindigs first(I recommend to install it system wide, you can install it to virtualenv but you will need to specify environment variable for reticulate to use this virtualenv)

Usefull Links:
* [Build package with R studio](https://support.rstudio.com/hc/en-us/articles/200486518-Customizing-Package-Build-Options)
* [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html)
* You also able to build package from command line, you can check it in .travis.yml file

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
All methods provided by this package can be found [here](./brainflow/R).
