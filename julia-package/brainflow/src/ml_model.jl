import JSON


AnyIntType = Union{Int8, Int32, Int64, Int128, Int}


@enum BrainFlowMetrics begin

    RELAXATION = 0
    CONCENTRATION = 1

end


@enum BrainFlowClassifiers begin

    REGRESSION = 0

end


mutable struct BrainFlowModelParams

    metric::Int32
    classifier::Int32
    file::String
    other_info::String

    function BrainFlowModelParams(metric::AnyIntType, classifier::AnyIntType)
        new(Int32(metric), Int32(classifier), "", "")
    end

end


struct MLModel

    input_json::String

    function MLModel(params::BrainFlowModelParams)
        new(JSON.json(params))
    end

end


function prepare(ml_model::MLModel)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:prepare, "MLModule.dll"), Cint, (Ptr{UInt8},), ml_model.input_json)
    elseif Sys.isapple()
        ec = ccall((:prepare, "libMLModule.dylib"), Cint, (Ptr{UInt8},), ml_model.input_json)
    else
        ec = ccall((:prepare, "libMLModule.so"), Cint, (Ptr{UInt8},), ml_model.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in prepare ", ec), ec))
    end
end


function release(ml_model::MLModel)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:release, "MLModule.dll"), Cint, (Ptr{UInt8},), ml_model.input_json)
    elseif Sys.isapple()
        ec = ccall((:release, "libMLModule.dylib"), Cint, (Ptr{UInt8},), ml_model.input_json)
    else
        ec = ccall((:release, "libMLModule.so"), Cint, (Ptr{UInt8},), ml_model.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in release ", ec), ec))
    end
end


function predict(data, ml_model::MLModel)
    val = Vector{Float64}(undef, 1)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:predict, "MLModule.dll"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{UInt8}),
            data, length(data), val, ml_model.input_json)
    elseif Sys.isapple()
        ec = ccall((:predict, "libMLModule.dylib"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{UInt8}),
            data, length(data), val, ml_model.input_json)
    else
        ec = ccall((:predict, "libMLModule.so"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{UInt8}),
            data, length(data), val, ml_model.input_json)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in predict ", ec), ec))
    end
    value = val[1]
    value
end
