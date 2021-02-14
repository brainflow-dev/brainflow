using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Text;


namespace brainflow
{
    /// <summary>
    ///  Describe model
    /// </summary>
    [DataContract]
    public class BrainFlowModelParams
    {
        /// <summary>
        /// metric to caclulate
        /// </summary>
        [DataMember]
        public int metric;
        /// <summary>
        /// classifier to use
        /// </summary>
        [DataMember]
        public int classifier;
        /// <summary>
        /// path to model file
        /// </summary>
        [DataMember]
        public string file;
        /// <summary>
        /// other info
        /// </summary>
        [DataMember]
        public string other_info;

        public BrainFlowModelParams (int metric, int classifier)
        {
            this.metric = metric;
            this.classifier = classifier;
            file = "";
            other_info = "";
        }

        public string to_json ()
        {
            // copypaste from https://docs.microsoft.com/en-us/dotnet/framework/wcf/feature-details/how-to-serialize-and-deserialize-json-data?redirectedfrom=MSDN
            var ms = new MemoryStream ();
            var ser = new DataContractJsonSerializer (typeof (BrainFlowModelParams));
            ser.WriteObject (ms, this);
            byte[] json = ms.ToArray ();
            ms.Close ();
            return Encoding.UTF8.GetString (json, 0, json.Length);
        }
    }
}
