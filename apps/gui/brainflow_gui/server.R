library(reticulate)
library(brainflow)
library(shiny)
library(shinyWidgets)
library(loggit)
library(ggplot2)
require(reshape2)

setLogFile("brainflow_gui.json")

brainflow_server <- function(input, output, session) {
    sync_interval <- 100
    reactive_values <- reactiveValues()
    reactive_values$is_streaming <- FALSE
    reactive_values$board <- NULL
    reactive_values$eeg_data <- NULL
    reactive_values$board_shim <- NULL

    observe({
        invalidateLater(sync_interval, session)
        if (reactive_values$is_streaming == FALSE || is.null(reactive_values$board_shim)) {
            reactive_values$eeg_data <- NULL
            return ()
        }
        tryCatch({
            raw_data <- get_current_board_data(reactive_values$board_shim, as.integer(20 * reactive_values$board["sampling"] * sync_interval / 1000))
            if (is.null(raw_data)) {
                return ()
            }
            data_handler <- get_data_handler(reactive_values$board["Type"], raw_data)
            if (input$preprocess == TRUE) {
                preprocess_data(data_handler, 3, input$bandpass[1], input$bandpass[2], FALSE)
            }
            else {
                remove_dc_offset(data_handler)
                notch_interference(data_handler)
            }
            prepared_data <- get_data(data_handler)
            eeg_cols <- grep("^eeg", names(prepared_data), value=TRUE)

            reactive_values$eeg_data <- prepared_data[c(eeg_cols, "timestamp")]
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
        ggplot(df, aes(timestamp,value)) + geom_line(aes(colour = series)) + facet_grid(series ~ ., scales = "free_y") +
            xlab("Time(sec)") + ylab("Value") + ggtitle("EEG")
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
                start_stream(reactive_values$board_shim, 3600 * reactive_values$board["sampling"])
                reactive_values$is_streaming <- TRUE
                sendSweetAlert(session = session, title = "Started", text = "Started capturing EEG data",
                               type = "success", closeOnClickOutside = TRUE)
            }, error = function(e) {
                error_msg <- strsplit(x = e$message, split = "Detailed traceback")[[1]][1]
                sendSweetAlert(session = session, title = "Error", text = error_msg, type = "error", closeOnClickOutside = TRUE)
                loggit(log_lvl = "ERROR", e)
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
            raw_data <- get_board_data(reactive_values$board_shim)
            data_handler <- get_data_handler(reactive_values$board["Type"], raw_data)
            data <- get_data(data_handler)
            if (input$descr != "") {
                data$description <- input$descr
            }
            write.csv(data, file = input$file, append = TRUE)
            release_session(reactive_values$board_shim)
            reactive_values$is_streaming <- FALSE
            reactive_values$board_shim <- NULL
            reactive_values$board <- NULL
            sendSweetAlert(session = session, title = "Stopped", text = "Stop capturing EEG data",
                               type = "success", closeOnClickOutside = TRUE)
        }, error = function(e) {
            error_msg <- strsplit(x = e$message, split = "Detailed traceback")[[1]][1]
            sendSweetAlert(session = session, title = "Error", text = error_msg, type = "error", closeOnClickOutside = TRUE)
            loggit(log_lvl = "ERROR", e)
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
