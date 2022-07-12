classdef LogLevels < int32
    % Store all possible log levels
    enumeration
        LEVEL_TRACE(0)
        LEVEL_DEBUG(1)
        LEVEL_INFO(2)
        LEVEL_WARN(3)
        LEVEL_ERROR(4)
        LEVEL_CRITICAL(5)
        LEVEL_OFF(6)
    end
end