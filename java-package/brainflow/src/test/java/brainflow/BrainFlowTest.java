package brainflow;

public class BrainFlowTest {

    public static void main (String[] args) throws Exception {

        BoardShim.enable_board_logger ();
        BoardShim.set_log_file ("test_log.txt");
        BoardShim board_shim = new BoardShim (Integer.parseInt (args[0]), args[1]);
        board_shim.prepare_session ();
        System.out.println ("Session is ready");
        board_shim.start_stream (3600);
        System.out.println ("Started");
        Thread.sleep (1000);
        board_shim.stop_stream ();
        System.out.println ("Stopped");
        System.out.println (board_shim.get_board_data_count ());
        System.out.println (board_shim.get_board_data ());
        board_shim.release_session ();
        System.out.println ("Released");
    }
}
