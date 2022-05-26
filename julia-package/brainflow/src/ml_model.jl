import JSON
export BrainFlowModelParams

abstract type BrainFlowMetric end
struct Mindfulness <: BrainFlowMetric end
struct Restfulness <: BrainFlowMetric end
struct UserDefined <: BrainFlowMetric end
Base.Integer(::Mindfulness) = 0
Base.Integer(::Restfulness) = 1
Base.Integer(::UserDefined) = 2
const MINDFULNESS = Mindfulness()
const RESTFULNESS = Restfulness()
const USER_DEFINED = UserDefined()

abstract type BrainFlowClassifier end
struct DefaultClassifier <: BrainFlowClassifier end
struct DynLibClassifier <: BrainFlowClassifier end
struct ONNXClassifier <: BrainFlowClassifier end
Base.Integer(::DefaultClassifier) = 0
Base.Integer(::DynLibClassifier) = 1
Base.Integer(::ONNXClassifier) = 2
const DEFAULT_CLASSIFIER = DefaultClassifier()
const DYN_LIB_CLASSIFIER = DynLibClassifier()
const ONNX_CLASSIFIER = ONNXClassifier()

@Base.kwdef mutable struct BrainFlowModelParams
    metric::BrainFlowMetric = MINDFULNESS
    classifier::BrainFlowClassifier = DEFAULT_CLASSIFIER
    file::String = ""
    other_info::String = ""
    output_name::String = ""
    max_array_size::Int32 = 8192
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
    value = val[1:len[1]]
    return value
end

@brainflow_rethrow function release_all()
    ccall((:release_all, ML_MODULE_INTERFACE), Cint, ())
end