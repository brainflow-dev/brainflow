package brainflow;

import com.google.gson.Gson;

public class BrainFlowInputParams
{

    // wo get/set to have an API like in other languages
    public String ip_address;
    public String mac_address;
    public String serial_port;
    public int ip_port;
    public int ip_protocol;
    public String other_info;

    public BrainFlowInputParams ()
    {
        ip_address = "";
        mac_address = "";
        serial_port = "";
        ip_port = 0;
        ip_protocol = IpProtocolType.NONE.get_code ();
        other_info = "";
    }

    public String to_json ()
    {
        return new Gson ().toJson (this);
    }
}
