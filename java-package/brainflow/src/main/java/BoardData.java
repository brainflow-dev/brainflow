import java.util.ArrayList;
import java.util.List;


public class BoardData {
	
	private List<ArrayList<Double>> board_data = new ArrayList<ArrayList<Double>> ();
	private int num_channels;
	
	public BoardData (int num_channels, float[] raw_data, double[] ts_data) {
		this.num_channels = num_channels;
		for (int i = 0; i < ts_data.length; i++) {
			ArrayList<Double> temp = new ArrayList<Double> ();
			for (int j = 0; j < this.num_channels; j++) {
				temp.add ((double) raw_data[i * num_channels + j]);
			}
			temp.add (ts_data[i]);
			board_data.add (temp);
		}
	}
	
	@Override
    public String toString() {
		StringBuilder str_b = new StringBuilder ();
		for (ArrayList<Double> row : board_data) {
			for (Double data_point : row) {
				str_b.append (data_point).append (",");
			}
			str_b.append("\n");
		}
        return str_b.toString (); 
    }

	public List<ArrayList<Double>> get_board_data () {
		return board_data;
	}
	
	public int get_size () {
		return board_data.size ();
	}
	
	public int get_num_channels () {
		return num_channels;
	}

}
