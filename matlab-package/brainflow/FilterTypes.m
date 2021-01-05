classdef FilterTypes < int32
    % Store all possible filters
    enumeration
        BUTTERWORTH(0)
        CHEBYSHEV_TYPE_1(1)
        BESSEL(2)
    end
end