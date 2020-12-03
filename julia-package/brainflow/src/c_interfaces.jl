function interface_path(library::AbstractString)
    return abspath(INTERFACE_PATH, interface_name(library))
end

function interface_name(library::AbstractString)
    if Sys.iswindows()
        return "$library.dll"
    elseif Sys.isapple()
        return "lib$library.dylib"
    else
        return "lib$library.so"
    end
end

# We can later replace this with:
# using Pkg.Artifacts
# const INTERFACE_PATH = artifact"brainflow"
# Right now using a hardcoded path during refactoring, with manual /lib location
const INTERFACE_PATH = abspath(joinpath(@__DIR__, "../lib"))

# we should instead use dlopen(interface_path())?
const DATA_HANDLER_INTERFACE = interface_path("DataHandler")
const BOARD_CONTROLLER_INTERFACE = interface_path("BoardController")
const ML_MODULE_INTERFACE = interface_path("MLModule")
