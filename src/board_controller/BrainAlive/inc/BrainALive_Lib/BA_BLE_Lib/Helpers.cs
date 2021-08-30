using System;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;

namespace BrainAlive_Lib
{

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct BoardData
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 20)]
        public byte[] data;
        public long timestamp;
        public int exit_code;
    }

    public class BrainAliveConstants
    {
        // name format is BrainALive-....
        public const string name_prefix = "BrainAlive";
        // from time to time watcher receives Simblee as a BrainAlive name
        public const string alt_name = "BA_SRV";
        public const string service_id = "0000fe40-cc7a-482a-984a-7f2ed5b3e58f";
        public const string characteristic_send_id = "0000fe41-8e22-4541-9d4c-21edae82ed19";
        public const string characteristic_receive_id = "0000fe41-8e22-4541-9d4c-21edae82ed19";
        public const string characteristic_disconnect_id = "0000fe41-8e22-4541-9d4c-21edae82ed19";
        public const int sampling_rate = 250;
    }

    public enum CustomExitCodes
    {
        STATUS_OK = 0,
        BRAINALIVE_NOT_FOUND_ERROR,
        BRAINALIVE_IS_NOT_OPEN_ERROR,
        BRAINALIVE_ALREADY_PAIR_ERROR,
        BRAINALIVE_IS_NOT_PAIRED_ERROR,
        BRAINALIVE_ALREADY_OPEN_ERROR,
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
        public static async Task<TResult> TimeoutAfter<TResult>(this Task<TResult> task, TimeSpan timeout)
        {
            using (var timeoutCancellationTokenSource = new CancellationTokenSource())
            {
                var completedTask = await Task.WhenAny(task, Task.Delay(timeout, timeoutCancellationTokenSource.Token));
                if (completedTask == task)
                {
                    timeoutCancellationTokenSource.Cancel();
                    return await task;
                }
                else
                {
                    throw new TimeoutException("The operation has timed out.");
                }
            }
        }
    }

}
