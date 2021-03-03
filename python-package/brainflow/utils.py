import numpy as np

from brainflow.board_shim import BrainFlowError
from brainflow.exit_codes import BrainflowExitCodes


def check_memory_layout_row_major(data):
    if not data:
        raise BrainFlowError('empty data', BrainflowExitCodes.EMPTY_BUFFER_ERROR.value)
    if not data.flags['C_CONTIGUOUS']:
        raise BrainFlowError('wrong memory layout, should be row major, make sure you didnt tranpose array',
                             BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)