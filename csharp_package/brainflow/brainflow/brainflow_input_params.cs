using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;


namespace brainflow
{
    /// <summary>
    ///  Check SuportedBoards to get information about fields which are required for specific board
    /// </summary>
    [DataContract]
    public class BrainFlowInputParams
    {
        /// <summary>
        /// serial port name
        /// </summary>
        [DataMember]
        public string serial_port;
        /// <summary>
        /// MAC address
        /// </summary>
        [DataMember]
        public string mac_address;
        /// <summary>
        /// IP address
        /// </summary>
        [DataMember]
        public string ip_address;
        /// <summary>
        /// IP address aux
        /// </summary>
        [DataMember]
        public string ip_address_aux;
        /// <summary>
        /// IP address anc
        /// </summary>
        [DataMember]
        public string ip_address_anc;
        /// <summary>
        /// PORT
        /// </summary>
        [DataMember]
        public int ip_port;
        /// <summary>
        /// PORT
        /// </summary>
        [DataMember]
        public int ip_port_aux;
        /// <summary>
        /// PORT
        /// </summary>
        [DataMember]
        public int ip_port_anc;
        /// <summary>
        /// IP protocol, use IpProtocolTypes
        /// </summary>
        [DataMember]
        public int ip_protocol;
        /// <summary>
        /// you can provide additional info to low level API using this field
        /// </summary>
        [DataMember]
        public string other_info;
        /// <summary>
        /// timeout for device discovery or connection
        /// </summary>
        [DataMember]
        public int timeout;
        /// <summary>
        /// serial number
        /// </summary>
        [DataMember]
        public string serial_number;
        /// <summary>
        /// file
        /// </summary>
        [DataMember]
        public string file;
        /// <summary>
        /// file
        /// </summary>
        [DataMember]
        public string file_aux;
        /// <summary>
        /// file
        /// </summary>
        [DataMember]
        public string file_anc;
        /// <summary>
        /// Master board, use BoardIds enums
        /// </summary>
        [DataMember]
        public int master_board;

        public BrainFlowInputParams ()
        {
            serial_port = "";
            mac_address = "";
            ip_address = "";
            ip_address_aux = "";
            ip_address_anc = "";
            ip_port = 0;
            ip_port_aux = 0;
            ip_port_anc = 0;
            ip_protocol = (int)IpProtocolTypes.NO_IP_PROTOCOL;
            other_info = "";
            timeout = 0;
            serial_number = "";
            file = "";
            file_aux = "";
            file_anc = "";
            master_board = (int)BoardIds.NO_BOARD;
        }

        public string to_json ()
        {
            // copypaste from https://docs.microsoft.com/en-us/dotnet/framework/wcf/feature-details/how-to-serialize-and-deserialize-json-data?redirectedfrom=MSDN
            var ms = new MemoryStream ();
            var ser = new DataContractJsonSerializer (typeof (BrainFlowInputParams));
            ser.WriteObject (ms, this);
            byte[] json = ms.ToArray ();
            ms.Close ();
            return Encoding.UTF8.GetString (json, 0, json.Length);
        }
    }
}
