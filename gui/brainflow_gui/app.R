library(shiny)
source('ui.R', local = TRUE)
source('server.R')


shinyApp(
  ui = brainflow_ui,
  server = brainflow_server
)