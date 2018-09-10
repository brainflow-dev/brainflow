library(shiny)
library(shinydashboard)
library(shinyWidgets)


menu <- dropdownMenu(
    type = "notifications",
    badgeStatus = "warning",
    notificationItem(
        text = "Project page",
        icon("users"),
        href = "https://github.com/Andrey1994/brainflow"
    ),
    notificationItem(
        text = "Issues",
        icon("life-ring"),
        href = "https://github.com/Andrey1994/brainflow/issues"
    ),
    notificationItem(
        text = "Contact us",
        icon("address-book"),
        href = "a1994ndrey@gmail.com"
    )
)

sidebar <- dashboardSidebar(
    sidebarMenu(
        menuItem("OpenBCI", icon = icon("asterisk"), expandedName = "OpenBCI",
            selectInput(inputId = 'openbci_boards', label = 'Choose Board', choices = c("Cython"), multiple = FALSE),
            textInput("port", "OpenBCI Port", value = "/dev/ttyUSB0")
        ),
        textInput("descr", "Data description", placeholder = "(Optional)Describe your data"),
        textInput("file", "File to store data", value = "results.csv"),
        uiOutput("session_control"),
        sliderInput("bandpass", "Bandpass range", min = 1, max = 50, value = c(1, 30)),
        switchInput(inputId = "preprocess", label = "Preprocessing", value = TRUE)
    )

)

body <- dashboardBody(
    fluidRow(
        column(
            width = 6,
            box(
                title = "EEG Data",
                status = "warning",
                solidHeader = TRUE,
                plotOutput("eeg_data", height = "85vh"),
                width = NULL
            )
        ),

        column(
            width = 6,
            box(
                title = "FFT Data",
                status = "warning",
                solidHeader = TRUE,
                plotOutput("fft_data", height = "85vh"),
                width = NULL
            )
        )
    )
)

brainflow_ui <- dashboardPage(
    title = "Brainflow GUI",
    skin = "black",
    dashboardHeader(title = "Brainflow GUI", menu),
    sidebar,
    body
)
