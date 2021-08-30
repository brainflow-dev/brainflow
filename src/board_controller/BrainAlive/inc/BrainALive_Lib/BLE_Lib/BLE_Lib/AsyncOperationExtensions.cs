using System.Threading.Tasks;
using Windows.Foundation;

namespace BLE_Lib
{
    /// <summary>
    /// Provides helper methods for the <see cref="IAsyncOperation"/>
    /// </summary>
    public static class AsyncOperationExtensions
    {
        /// <summary>
        /// Convert an <see cref="IAsyncOperation{TResult}"/>
        /// into a <see cref="Task{TResult}"/>
        /// </summary>
        /// <typeparam name="TResult">The type of result expected</typeparam>
        /// <param name="operation">The Async Operation</param>
        /// <returns></returns>
        public static Task<TResult> AsTask<TResult>(this IAsyncOperation<TResult> operation)
        {
            // Create task completion result
            var tcs = new TaskCompletionSource<TResult>();

            // When the operation is completed...
            operation.Completed += delegate
            {
                switch (operation.Status)
                {
                    // If successful...
                    case AsyncStatus.Completed:
                        // Set result
                        tcs.TrySetResult(operation.GetResults());
                        break;
                    // If exception...
                    case AsyncStatus.Error:
                        // Set exception
                        tcs.TrySetException(operation.ErrorCode);
                        break;
                    // If canceled...
                    case AsyncStatus.Canceled:
                        // Set task as canceled
                        tcs.SetCanceled();
                        break;
                }
            };

            // Return the task
            return tcs.Task;
        }

        
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
}
