using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;

namespace brainflow
{
    public enum IpProtocolType
    {
        NONE = 0,
        UDP = 1,
        TCP = 2
    };

    [DataContract]
    public class BrainFlowInputParams
    {
        [DataMember]
        public string serial_port;
        [DataMember]
        public string mac_address;
        [DataMember]
        public string ip_address;
        [DataMember]
        public int ip_port;
        [DataMember]
        public int ip_protocol;
        [DataMember]
        public string other_info;

        public BrainFlowInputParams ()
        {
            serial_port = "";
            mac_address = "";
            ip_address = "";
            ip_port = 0;
            ip_protocol = (int)IpProtocolType.NONE;
            other_info = "";
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
