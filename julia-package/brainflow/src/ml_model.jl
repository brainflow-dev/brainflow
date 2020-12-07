import JSON


@enum BrainFlowMetrics begin

    RELAXATION = 0
    CONCENTRATION = 1

end


@enum BrainFlowClassifiers begin

    REGRESSION = 0
    KNN = 1
    SVM = 2
    LDA = 3

end


mutable struct BrainFlowModelParams

    metric::Int32
    classifier::Int32
    file::String
    other_info::String

    function BrainFlowModelParams(metric::Integer, classifier::Integer)
        new(Int32(metric), Int32(classifier), "", "")
    end

end


struct MLModel

    input_json::String

    function MLModel(params::BrainFlowModelParams)
        new(JSON.json(params))
    end

end

@brainflow_rethrow function prepare(ml_model::MLModel)
    ccall((:prepare, ML_MODULE_INTERFACE), Cint, (Ptr{UInt8},), ml_model.input_json)
    return
end

@brainflow_rethrow function release(ml_model::MLModel)
    ccall((:release, ML_MODULE_INTERFACE), Cint, (Ptr{UInt8},), ml_model.input_json)
    return
end

@brainflow_rethrow function predict(data, ml_model::MLModel)
    val = Vector{Float64}(undef, 1)
    ccall((:predict, ML_MODULE_INTERFACE), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Ptr{UInt8}),
        data, length(data), val, ml_model.input_json)
    value = val[1]
    return value
end
