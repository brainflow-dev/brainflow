using System;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;

namespace GanglionLib
{
    [StructLayout (LayoutKind.Sequential, Pack = 1)]
    public struct BoardData
    {
        [MarshalAs (UnmanagedType.ByValArray, SizeConst = 20)]
        public byte[] data;
        public long timestamp;
        public int exit_code;
    }

    public class GanglionConstants
    {
        // name format is Ganglion-....
        public const string name_prefix = "Ganglion";
        // from time to time watcher receives Simblee as a Ganglion name
        public const string alt_name = "Simblee";
        public const string service_id = "0000fe84-0000-1000-8000-00805f9b34fb";
        public const string characteristic_send_id = "2d30c083-f39f-4ce6-923f-3484ea480596";
        public const string characteristic_receive_id = "2d30c082-f39f-4ce6-923f-3484ea480596";
        public const string characteristic_disconnect_id = "2d30c084-f39f-4ce6-923f-3484ea480596";
        public const int sampling_rate = 200;
    }

    public enum CustomExitCodes
    {
        STATUS_OK = 0,
        GANGLION_NOT_FOUND_ERROR,
        GANGLION_IS_NOT_OPEN_ERROR,
        GANGLION_ALREADY_PAIR_ERROR,
        GANGLION_ALREADY_OPEN_ERROR,
        SERVICE_NOT_FOUND_ERROR,
        SEND_CHARACTERISTIC_NOT_FOUND_ERROR,
        RECEIVE_CHARACTERISTIC_NOT_FOUND_ERROR,
        DISCONNECT_CHARACTERISTIC_NOT_FOUND_ERROR,
        TIMEOUT_ERROR,
        STOP_ERROR,
        FAILED_TO_SET_CALLBACK_ERROR,
        FAILED_TO_UNSUBSCRIBE_ERROR,
        GENERAL_ERROR,
        NO_DATA_ERROR
    };

    public static class TaskExtensions
    {
        public static async Task<TResult> TimeoutAfter<TResult> (this Task<TResult> task, TimeSpan timeout)
        {
            using (var timeoutCancellationTokenSource = new CancellationTokenSource ())
            {
                var completedTask = await Task.WhenAny (task, Task.Delay (timeout, timeoutCancellationTokenSource.Token));
                if (completedTask == task)
                {
                    timeoutCancellationTokenSource.Cancel ();
                    return await task;
                }
                else
                {
                    throw new TimeoutException ("The operation has timed out.");
                }
            }
        }
    }
}