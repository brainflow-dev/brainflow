# Matlab

#### TODO list:
* test on all platforms
* convert it to package

## Example:
```
board_shim = BoardShim (BoardsIds.CYTON_BOARD, '/dev/ttyUSB0')
ec = board_shim.prepare_session ()
board_shim.check_ec (ec)
ec = board_shim.start_stream (3600)
board_shim.check_ec (ec)
pause (5)
ec = board_shim.stop_stream ()
board_shim.check_ec (ec)
[ec, data, ts] = board_shim.get_current_board_data (250)
board_shim.check_ec (ec)
disp (data)
ec = board_shim.release_session ()
board_shim.check_ec (ec)
```
[BoardShim matlab class](./brainflow/BoardShim.m)
