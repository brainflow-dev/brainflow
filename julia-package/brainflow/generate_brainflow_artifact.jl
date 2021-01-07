# when new brainflow binaries are released, then this script needs to be re-executed.

using SHA
using Pkg
using Pkg.Artifacts
Pkg.add("Tar")
using Tar

function sha256sum(tarball_path)
    return open(tarball_path, "r") do io
        return bytes2hex(sha256(io))
    end
end

function add_brainflow_artifact!(
    url,
    artifact_toml_path = "Artifacts.toml",
    artifact_name = "brainflow",
    )
    
    download_path = "$(tempname())-download.tar"
    download(url, download_path)
    tar_hash_sha256 = sha256sum(download_path)

    brainflow_hash = create_artifact() do artifact_dir 
        # Pkg.PlatformEngines.unpack() gives errors for some users on Windows
        Tar.extract(download_path, artifact_dir)
    end

    rm(download_path)

    Pkg.Artifacts.bind_artifact!(
        artifact_toml_path, 
        artifact_name, 
        brainflow_hash; 
        download_info=[(url, tar_hash_sha256)], 
        force=true,
        lazy=true,
    )

    return brainflow_hash
end

cd(@__DIR__)
include("src/brainflow_url.jl")
add_brainflow_artifact!(brainflow_url())
