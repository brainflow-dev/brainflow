package brainflow;

public class BrainFlowTest {

    public static void main (String[] args) throws Exception {

        BoardShim.enable_board_logger ();
        BoardShim board_shim = new BoardShim (Integer.parseInt (args[0]), args[1]);
        board_shim.prepare_session ();
        System.out.println ("Session is ready");
        board_shim.start_stream (3600);
        System.out.println ("Started");
        Thread.sleep (10000);
        board_shim.stop_stream ();
        System.out.println ("Stopped");
        if (board_shim.get_board_data_count () < 100)
        {
            System.out.println ("too small data");
            throw new IllegalStateException ();
        }
        System.out.println (board_shim.get_board_data_count ());
        board_shim.release_session ();
        System.out.println ("Released");
    }
}
