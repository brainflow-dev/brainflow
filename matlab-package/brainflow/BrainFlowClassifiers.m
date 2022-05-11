classdef BrainFlowClassifiers < int32
    % Store supported classifiers
    enumeration
        REGRESSION(0)
        KNN(1)
        SVM(2)
        LDA(3)
        DYN_LIB_CLASSIFIER(4)
    end
end