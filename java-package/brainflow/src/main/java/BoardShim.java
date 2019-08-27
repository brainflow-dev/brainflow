package brainflow;

import java.io.File;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.io.IOException;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.Map;

import org.apache.commons.lang3.SystemUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class BoardShim {

    public interface DllInterface extends Library {
        int prepare_session (int board_id, String port_name);
        int config_board (String port_name);
        int start_stream (int buffer_size);
        int stop_stream ();
        int release_session ();
        int get_current_board_data (int num_samples, float [] data_buf, double [] ts_buf, int[] returned_samples);
        int get_board_data_count (int[] result);
        int get_board_data (int data_count, float[] data_buf, double[] ts_buf);
    }

    public int package_length;
    public int board_id;
    public String port_name;
    public DllInterface instance;
    public BoardShim (int board_id, String port_name, Boolean unpack_lib) throws BrainFlowError, IOException, ReflectiveOperationException {
        this.board_id = board_id;
        package_length = BoardInfoGetter.get_package_length(board_id);
        
        String lib_name = "libBoardController.so";
        if (SystemUtils.IS_OS_WINDOWS)
        {
            lib_name = "BoardController.dll";
            
        }
        else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libBoardController.dylib";
        }

        if (unpack_lib)
        {
            // need to extract libraries from jar
            unpack_from_jar (lib_name);
            if (SystemUtils.IS_OS_WINDOWS)
            {
                unpack_from_jar ("GanglionLib.dll");
                unpack_from_jar ("GanglionLibNative64.dll");
            }
        }
        this.instance = (DllInterface) Native.loadLibrary (lib_name, DllInterface.class);
        this.port_name = port_name;
    }
    
    private void unpack_from_jar (String lib_name) throws IOException
    {
        File file = new File (lib_name);
        if (file.exists ())
            file.delete ();
        InputStream link = (getClass().getResourceAsStream (lib_name));
        Files.copy (link, file.getAbsoluteFile ().toPath ());
    }

    public void prepare_session () throws BrainFlowError {
        System.out.println(board_id + port_name);
        int ec = this.instance.prepare_session (board_id, port_name);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in prepare_session", ec);
        }
    }

    public void config_board (String config) throws BrainFlowError {
        int ec = this.instance.config_board (config);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in config_board", ec);
        }
    }

    public void start_stream (int buffer_size) throws BrainFlowError {
        int ec = this.instance.start_stream (buffer_size);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in start_stream", ec);
        }
    }

    public void stop_stream () throws BrainFlowError {
        int ec = this.instance.stop_stream ();
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in stop_stream", ec);
        }
    }

    public void release_session () throws BrainFlowError {
        int ec = this.instance.release_session ();
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in release_session", ec);
        }
    }

    public int get_board_data_count () throws BrainFlowError {
        int[] res = new int[1];
        int ec = this.instance.get_board_data_count (res);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in get_board_data_count", ec);
        }
        return res[0];
    }

    public BoardData get_current_board_data (int num_samples) throws BrainFlowError {
        float[] data_arr = new float[num_samples * package_length];
        double[] ts_arr = new double[num_samples];
        int[] current_size = new int[1];
        int ec = this.instance.get_current_board_data (num_samples, data_arr, ts_arr, current_size);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in get_current_board_data", ec);
        }
        return new BoardData (package_length, Arrays.copyOfRange(data_arr, 0, current_size[0] * package_length),
                Arrays.copyOfRange(ts_arr, 0, current_size[0]));
    }

    public BoardData get_immediate_board_data () throws BrainFlowError {
        return get_current_board_data (0);
    }

    public BoardData get_board_data () throws BrainFlowError {
        int size = get_board_data_count ();
        float[] data_arr = new float[size * package_length];
        double[] ts_arr = new double[size];
        int ec = this.instance.get_board_data (size, data_arr, ts_arr);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in get_board_data", ec);
        }
        return new BoardData (package_length, data_arr, ts_arr);
    }

}
