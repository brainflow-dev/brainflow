import JSON
export BrainFlowModelParams

@enum BrainFlowMetrics begin

    MINDFULNESS = 0
    RESTFULNESS = 1
    USER_DEFINED = 2

end

MetricType = Union{BrainFlowMetrics, Integer}

@enum BrainFlowClassifiers begin

    DEFAULT_CLASSIFIER = 0
    DYN_LIB_CLASSIFIER = 1
    ONNX_CLASSIFIER = 2

end

ClassifierType = Union{BrainFlowClassifiers, Integer}

mutable struct BrainFlowModelParams
    metric::MetricType
    classifier::ClassifierType
    file::String
    other_info::String
    output_name::String
    max_array_size::Int32

    function BrainFlowModelParams(metric_::MetricType, classifier_::ClassifierType)
        new(metric_, classifier_, "", "", "", 8192)
    end

end

function JSON.json(params::BrainFlowModelParams)
    if params.max_array_size < 1
        throw(BrainFlowError("max_array_size must be positive", Integer(INVALID_ARGUMENTS_ERROR)))
    end
    d = Dict(
        "metric" => Integer(params.metric), 
        "classifier" => Integer(params.classifier), 
        "file" => params.file, 
        "other_info" => params.other_info,
        "output_name" => params.output_name,
        "max_array_size" => params.max_array_size, 
        )
    return JSON.json(d)
end

@brainflow_rethrow function prepare(params::BrainFlowModelParams)
    input_json = JSON.json(params)
    ccall((:prepare, ML_MODULE_INTERFACE), Cint, (Ptr{UInt8},), input_json)
    return
end

@brainflow_rethrow function release(params::BrainFlowModelParams)
    input_json = JSON.json(params)
    ccall((:release, ML_MODULE_INTERFACE), Cint, (Ptr{UInt8},), input_json)
    return
end

@brainflow_rethrow function predict(data, params::BrainFlowModelParams)
    # Keep the native lookup key and allocated capacity from the same snapshot.
    snapshot = deepcopy(params)
    input_json = JSON.json(snapshot)
    capacity = Int(snapshot.max_array_size)
    if !(data isa AbstractVector{<:Real}) || !(0 < length(data) <= typemax(Cint))
        throw(BrainFlowError("Expected a nonempty real feature vector", Integer(INVALID_ARGUMENTS_ERROR)))
    end
    data = Vector{Float64}(data)
    if !all(isfinite, data)
        throw(BrainFlowError("Feature values must be finite", Integer(INVALID_ARGUMENTS_ERROR)))
    end
    val = Vector{Float64}(undef, capacity)
    val_len = Ref{Cint}(0)
    ccall((:predict, ML_MODULE_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{Cint}, Ptr{UInt8}),
        data, length(data), val, val_len, input_json)
    if !(0 <= val_len[] <= capacity)
        throw(BrainFlowError("Native prediction length exceeds the output buffer", Integer(GENERAL_ERROR)))
    end
    value = val[1:Int(val_len[])]
    return value
end

@brainflow_rethrow function release_all()
    ccall((:release_all, ML_MODULE_INTERFACE), Cint, ())
end
