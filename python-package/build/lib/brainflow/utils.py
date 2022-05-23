from brainflow.board_shim import BrainFlowError
from brainflow.exit_codes import BrainflowExitCodes


def check_memory_layout_row_major(data, ndim):
    if data is None:
        raise BrainFlowError('data is None',
                             BrainflowExitCodes.EMPTY_BUFFER_ERROR.value)
    if len(data.shape) != ndim:
        raise BrainFlowError('wrong shape for filter data array, it should be %dd array' % ndim,
                             BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
    if not data.flags['C_CONTIGUOUS']:
        raise BrainFlowError('wrong memory layout, should be row major, make sure you didnt tranpose array',
                             BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
