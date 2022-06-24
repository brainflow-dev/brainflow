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
    input_json = JSON.json(params)
    val = Vector{Float64}(undef, params.max_array_size)
    val_len = Vector{Float64}(undef, 1)
    ccall((:predict, ML_MODULE_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{Cint}, Ptr{UInt8}),
        data, length(data), val, val_len, input_json)
    value = val[1:val_len[1]]
    return value
end

@brainflow_rethrow function release_all()
    ccall((:release_all, ML_MODULE_INTERFACE), Cint, ())
end