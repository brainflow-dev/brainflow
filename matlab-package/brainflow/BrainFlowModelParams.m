classdef BrainFlowModelParams
    % Store MLModel params
    properties
        metric
        classifier
        file
        other_info
    end
    methods
        function obj = BrainFlowModelParams(metric, classifier)
            obj.metric = metric;
            obj.classifier = classifier;
            obj.file = '';
            obj.other_info = '';
        end
        function json_string = to_json(obj)
            json_string = jsonencode(obj);
        end
    end
end