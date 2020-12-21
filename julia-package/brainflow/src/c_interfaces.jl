using Pkg
using Pkg.Artifacts
using SHA
using Tar

# we have an issue with unpack(), so wrote a custom download
# https://discourse.julialang.org/t/unable-to-automatically-install-artifact/51984/2
function download_brainflow_artifact()

    url = brainflow_url()

    download_path = "$(tempname())-download.tar"
    download(url, download_path)

    brainflow_hash = create_artifact() do artifact_dir 
        # Pkg.PlatformEngines.unpack() gives errors for some users on Windows
        Tar.extract(download_path, artifact_dir)
    end

    rm(download_path)

    return brainflow_hash
end

# library path you can optionally use while developing
function dev_library_path()
    return abspath(joinpath(dirname(@__DIR__), "lib"))
end

function get_brainflow_artifact_path()
    artifacts_toml = find_artifacts_toml(@__DIR__) # is there a better way? @__DIR__ makes brainflow non-relocatable I believe.
    brainflow_hash = artifact_hash("brainflow", artifacts_toml)
    if isdir(dev_library_path())
        # developer library takes precedence
        println(string("Using local libraries: ", dev_library_path()))
        return dev_library_path()
    elseif artifact_exists(brainflow_hash)
        # libraries are automatically stored here for users of brainflow
        return artifact_path(brainflow_hash)
    else
        println("Downloading artifact: brainflow")
        brainflow_hash = download_brainflow_artifact()
        return artifact_path(brainflow_hash)
    end
end

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

const INTERFACE_PATH = get_brainflow_artifact_path()

const DATA_HANDLER_INTERFACE = interface_path("DataHandler")
const BOARD_CONTROLLER_INTERFACE = interface_path("BoardController")
const ML_MODULE_INTERFACE = interface_path("MLModule")
