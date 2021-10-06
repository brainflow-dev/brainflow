
try BoardShim.enableDevBoardLogger()
let params = BrainFlowInputParams()
let board = try BoardShim(.SYNTHETIC_BOARD, params)

print("Hello, world!")
