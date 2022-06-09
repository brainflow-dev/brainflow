classdef BrainFlowModelParams
    % Store MLModel params
    properties
        metric
        classifier
        file
        other_info
        output_name
        max_array_size
    end
    methods
        function obj = BrainFlowModelParams(metric, classifier)
            obj.metric = metric;
            obj.classifier = classifier;
            obj.file = '';
            obj.other_info = '';
            obj.output_name = '';
            obj.max_array_size = 8192;
        end
        function json_string = to_json(obj)
            json_string = jsonencode(obj);
        end
    end
end