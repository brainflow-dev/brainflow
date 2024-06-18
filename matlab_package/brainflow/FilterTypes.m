classdef FilterTypes < int32
    % Store all possible filters
    enumeration
        BUTTERWORTH(0)
        CHEBYSHEV_TYPE_1(1)
        BESSEL(2)
        BUTTERWORTH_ZERO_PHASE(3)
        CHEBYSHEV_TYPE_1_ZERO_PHASE(4)
        BESSEL_ZERO_PHASE(5)
    end
end