import JSON


AnyIntType = Union{Int8, Int32, Int64, Int128, Int}


@enum BrainFlowMetrics begin

    RELAXATION = 0
    CONCENTRATION = 1

end


@enum BrainFlowClassifiers begin

    ALGORITHMIC = 0
    REGRESSION = 1
    SVM = 2

end


struct MLModel

    metric::Int32
    classifier::Int32

    function MLModel(_metric::AnyIntType, _classifier::AnyIntType)
        new(_metric, _classifier)
    end

end


function prepare(ml_model::MLModel)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:prepare, "MLModule.dll"), Cint, (Cint, Cint), ml_model.metric, ml_model.classifier)
    elseif Sys.isapple()
        ec = ccall((:prepare, "libMLModule.dylib"), Cint, (Cint, Cint), ml_model.metric, ml_model.classifier)
    else
        ec = ccall((:prepare, "libMLModule.so"), Cint, (Cint, Cint), ml_model.metric, ml_model.classifier)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in prepare ", ec), ec))
    end
end


function release(ml_model::MLModel)
    ec = STATUS_OK
    # due to this bug https://github.com/JuliaLang/julia/issues/29602 libname should be hardcoded
    if Sys.iswindows()
        ec = ccall((:release, "MLModule.dll"), Cint, (Cint, Cint), ml_model.metric, ml_model.classifier)
    elseif Sys.isapple()
        ec = ccall((:release, "libMLModule.dylib"), Cint, (Cint, Cint), ml_model.metric, ml_model.classifier)
    else
        ec = ccall((:release, "libMLModule.so"), Cint, (Cint, Cint), ml_model.metric, ml_model.classifier)
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
        ec = ccall((:predict, "MLModule.dll"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Cint, Cint),
            data, length(data), val, ml_model.metric, ml_model.classifier)
    elseif Sys.isapple()
        ec = ccall((:predict, "libMLModule.dylib"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Cint, Cint),
            data, length(data), val, ml_model.metric, ml_model.classifier)
    else
        ec = ccall((:predict, "libMLModule.so"), Cint, (Ptr{Float64}, Cint, Ptr{Float64}, Cint, Cint),
            data, length(data), val, ml_model.metric, ml_model.classifier)
    end
    if ec != Integer(STATUS_OK)
        throw(BrainFlowError(string("Error in predict ", ec), ec))
    end
    value = val[1]
    value
end
