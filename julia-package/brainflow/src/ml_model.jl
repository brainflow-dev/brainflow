import JSON
export BrainFlowModelParams

abstract type BrainFlowMetric end
struct Relaxation <: BrainFlowMetric end
struct Concentration <: BrainFlowMetric end
struct UserDefined <: BrainFlowMetric end
Base.Integer(::Relaxation) = 0
Base.Integer(::Concentration) = 1
Base.Integer(::UserDefined) = 2
Base.convert(::Type{BrainFlowMetric}, s::AbstractString) = BrainFlowMetric(Val(Symbol(lowercase(s))))
BrainFlowMetric(::Val{:relaxation}) = Relaxation()
BrainFlowMetric(::Val{:concentration}) = Concentration()
BrainFlowMetric(::Val{:userdefined}) = UserDefined()

abstract type BrainFlowClassifier end
struct Regression <: BrainFlowClassifier end
struct Knn <: BrainFlowClassifier end
struct Svm <: BrainFlowClassifier end
struct Lda <: BrainFlowClassifier end
struct DynLibClassifier <: BrainFlowClassifier end
Base.Integer(::Regression) = 0
Base.Integer(::Knn) = 1
Base.Integer(::Svm) = 2
Base.Integer(::Lda) = 3
Base.Integer(::DynLibClassifier) = 4
Base.convert(::Type{BrainFlowClassifier}, s::AbstractString) = BrainFlowClassifier(Val(Symbol(lowercase(s))))
BrainFlowClassifier(::Val{:regression}) = Regression()
BrainFlowClassifier(::Val{:knn}) = Knn()
BrainFlowClassifier(::Val{:svm}) = Svm()
BrainFlowClassifier(::Val{:lda}) = Lda()
BrainFlowClassifier(::Val{:dynlibclassifier}) = DynLibClassifier()

@Base.kwdef mutable struct BrainFlowModelParams
    metric::BrainFlowMetric = Relaxation()
    classifier::BrainFlowClassifier = Regression()
    file::String = ""
    other_info::String = ""
end

function JSON.json(params::BrainFlowModelParams)
    d = Dict(
        "metric" => Integer(params.metric), 
        "classifier" => Integer(params.classifier), 
        "file" => params.file, 
        "other_info" => params.other_info, 
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
    val = Vector{Float64}(undef, 1)
    ccall((:predict, ML_MODULE_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{UInt8}),
        data, length(data), val, input_json)
    value = val[1]
    return value
end
