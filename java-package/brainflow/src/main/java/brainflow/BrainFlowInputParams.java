package brainflow;

import com.google.gson.Gson;

/**
 * to get fields which are required for your board check SupportedBoards section
 */
public class BrainFlowInputParams
{
    public String ip_address;
    public String mac_address;
    public String serial_port;
    public int ip_port;
    public int ip_protocol;
    public String other_info;
    public int timeout;
    public String serial_number;
    public String file;

    public BrainFlowInputParams ()
    {
        ip_address = "";
        mac_address = "";
        serial_port = "";
        ip_port = 0;
        ip_protocol = IpProtocolType.NONE.get_code ();
        other_info = "";
        timeout = 0;
        serial_number = "";
        file = "";
    }

    public String to_json ()
    {
        return new Gson ().toJson (this);
    }

    public String get_ip_address ()
    {
        return ip_address;
    }

    public void set_ip_address (String ip_address)
    {
        this.ip_address = ip_address;
    }

    public String get_mac_address ()
    {
        return mac_address;
    }

    public void set_mac_address (String mac_address)
    {
        this.mac_address = mac_address;
    }

    public String get_serial_port ()
    {
        return serial_port;
    }

    public void set_serial_port (String serial_port)
    {
        this.serial_port = serial_port;
    }

    public int get_ip_port ()
    {
        return ip_port;
    }

    public void set_ip_port (int ip_port)
    {
        this.ip_port = ip_port;
    }

    public int get_ip_protocol ()
    {
        return ip_protocol;
    }

    public void set_ip_protocol (int ip_protocol)
    {
        this.ip_protocol = ip_protocol;
    }

    public String get_other_info ()
    {
        return other_info;
    }

    public void set_other_info (String other_info)
    {
        this.other_info = other_info;
    }

    public void set_timeout (int timeout)
    {
        this.timeout = timeout;
    }

    public int get_timeout ()
    {
        return timeout;
    }

    public String get_serial_number ()
    {
        return serial_number;
    }

    public void set_serial_number (String serial_number)
    {
        this.serial_number = serial_number;
    }

    public String get_file ()
    {
        return file;
    }

    public void set_file (String file)
    {
        this.file = file;
    }
}
