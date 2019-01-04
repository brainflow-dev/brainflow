using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace brainflow
{
    public class BrainFlowExceptioin : Exception
    {
        public BrainFlowExceptioin (int code) : base (String.Format ("{0}:{1}", Enum.GetName (typeof (CustomExitCodes), code), code))
        {
        }
    }
}
