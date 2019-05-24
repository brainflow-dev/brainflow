# Java
Java binding for brainflow is a Maven project, which calls C methods using JNA

To compile it you just need to run:
```
mvn package
```

## Example:
```
BoardShim board_shim = new BoardShim (Boards.CYTON, "/dev/ttyUSB0", true);
board_shim.prepare_session ();
board_shim.start_stream (3600);
Thread.sleep (1000);
board_shim.stop_stream ();
System.out.println (board_shim.get_board_data_count ());
System.out.println (board_shim.get_board_data ());
board_shim.release_session ();
```
