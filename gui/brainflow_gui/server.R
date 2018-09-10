Sys.setenv(RETICULATE_PYTHON = "/home/andrey/venv_brainflow/bin/python")
library(reticulate)
library(brainflow)
library(shiny)
library(shinyWidgets)
library(loggit)
library(ggplot2)
require(reshape2)

setLogFile("brainflow_gui.json")


brainflow_server <- function(input, output, session) {
    sync_interval <- 200
    reactive_values <- reactiveValues()
    reactive_values$is_streaming <- FALSE
    reactive_values$board <- NULL
    reactive_values$eeg_data <- NULL
    reactive_values$fft_data <- NULL
    reactive_values$board_shim <- NULL
    reactive_values$data_handler <- NULL

    observe({
        invalidateLater(sync_interval, session)
        if (reactive_values$is_streaming == FALSE || is.null(reactive_values$board_shim)) {
            reactive_values$eeg_data <- NULL
            reactive_values$fft_data <- NULL
            return ()
        }
        tryCatch({
            raw_data <- get_current_board_data(reactive_values$board_shim, as.integer(2 * reactive_values$board["sampling"] * sync_interval / 1000))
            if (is.null(raw_data)) {
                return ()
            }
            data_handler <- get_data_handler(reactive_values$board["Type"], raw_data)
            if (input$preprocess == TRUE) {
                preprocess_data(data_handler, input$bandpass[1], input$bandpass[2], TRUE)
            }
            else {
                calculate_fft(data_handler)
            }
            prepared_data <- get_data(data_handler)
            eeg_cols <- grep("^eeg", names(prepared_data), value=TRUE)
            fft_cols <- grep("^fft", names(prepared_data), value=TRUE)

            reactive_values$eeg_data <- prepared_data[c(eeg_cols, "timestamp")]
            reactive_values$fft_data <- prepared_data[c(fft_cols, "timestamp")]
        }, error = function(e) {
            loggit(log_lvl = "ERROR", e)
        })

    })

    output$eeg_data <- renderPlot({
        invalidateLater(sync_interval, session)
        if (is.null(reactive_values$eeg_data)) {
            return ()
        }
        df <- melt(reactive_values$eeg_data,  id.vars = 'timestamp', variable.name = 'series')
        ggplot(df, aes(timestamp,value)) + geom_line(aes(colour = series)) + facet_grid(series ~ .)
    })

    output$fft_data <- renderPlot({
        invalidateLater(sync_interval, session)
        if (is.null(reactive_values$fft_data)) {
            return ()
        }
        #power_df <- sapply(reactive_values$fft_data, abs)
        #power_df <- data.frame(power_df)
        # average
        #power_df <- sapply(power_df, function(x) colMeans(matrix(x, nrow = 5)))
        #power_df <- data.frame(power_df)
        #df <- melt(power_df,  id.vars = 'timestamp', variable.name = 'series')
        #ggplot(df, aes(timestamp,value)) + geom_line(aes(colour = series))
    })

    output$session_control <- renderUI({
        if (reactive_values$is_streaming == TRUE) {
            actionBttn("stop_stream", label = "Stop Streaming", color = "danger", style = "bordered",
                       block = FALSE, icon = icon("unlink"), size = "md")
        }
        else {
            actionBttn("start_stream", label = "Start Streaming", color = "success", style = "bordered",
                       block = FALSE, icon = icon("sliders"), size = "md")
        }
    })

    observeEvent(input$start_stream, {
        if (is.null(input$sidebarItemExpanded)) {
            sendSweetAlert(session = session, title = "Board is not selected", text = "Please choose BCI board", type = "error", closeOnClickOutside = TRUE)
            return ()
        }
        if (input$sidebarItemExpanded == "OpenBCI") {
            tryCatch({
                if (input$openbci_boards == "Cython") {
                    reactive_values$board <- Boards()$Cython
                    reactive_values$board_shim <- get_board_shim(reactive_values$board["Type"], input$port)
                }
                else {
                    # for future
                    stopApp(returnValue = 1)
                }
                prepare_session(reactive_values$board_shim)
                start_stream(reactive_values$board_shim, 1800 * reactive_values$board["sampling"])
                reactive_values$is_streaming <- TRUE
                sendSweetAlert(session = session, title = "Started", text = "Start capturing EEG data",
                               type = "success", closeOnClickOutside = TRUE)
            }, error = function(e) {
                sendSweetAlert(session = session, title = "Error", text = e, type = "error", closeOnClickOutside = TRUE)
                reactive_values$is_streaming <- FALSE
                reactive_values$board_shim <- NULL
                reactive_values$board <- NULL
            })
        }
        else {
            #for future
            stopApp()
        }
    })

    observeEvent(input$stop_stream, {
        tryCatch({
            stop_stream(reactive_values$board_shim)
            data <- get_board_data(reactive_values$board_shim)
            write.csv(data, file = input$file, append = TRUE)
            release_session(reactive_values$board_shim)
            reactive_values$is_streaming <- FALSE
            reactive_values$board_shim <- NULL
            reactive_values$board <- NULL
            sendSweetAlert(session = session, title = "Stopped", text = "Stop capturing EEG data",
                               type = "success", closeOnClickOutside = TRUE)
        }, error = function(e) {
            sendSweetAlert(session = session, title = "Error", text = e, type = "error", closeOnClickOutside = TRUE)
            reactive_values$is_streaming <- TRUE
        })
    })

    cancel.onSessionEnded <- session$onSessionEnded(function() {
        shim <- isolate(reactive_values$board_shim)
        if (!(is.null(shim))) {
            release_session(shim)
        }
    })
}