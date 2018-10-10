# Java
Java binding for brainflow is a Maven project, which calls C methods using JNA

## Example:
```
BoardShim board_shim = new BoardShim (Boards.CYTHON, "/dev/ttyUSB0");
board_shim.prepare_session ();
board_shim.start_stream (3600);
Thread.sleep (1000);
board_shim.stop_stream ();
System.out.println (board_shim.get_board_data_count ());
System.out.println (board_shim.get_board_data ());
board_shim.release_session ();
```
Note: For some OS you may need to set jna.library.path properly
