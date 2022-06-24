# import time

from brainflow.data_filter import DataFilter, WindowOperations


def main():
    # demo for data windowing
    window_len = 20

    for window_function in range(4):

        if window_function == WindowOperations.NO_WINDOW.value:
            print('Window data for NO_WINDOW function:')
            window_data = DataFilter.get_window(WindowOperations.NO_WINDOW.value, window_len)
        elif window_function == WindowOperations.HANNING.value:
            print('Window data for HANNING function:')
            window_data = DataFilter.get_window(WindowOperations.HANNING.value, window_len)
        elif window_function == WindowOperations.HAMMING.value:
            print('Window data for HAMMING function:')
            window_data = DataFilter.get_window(WindowOperations.HAMMING.value, window_len)
        else:
            print('Window data for BLACKMAN_HARRIS function:')
            window_data = DataFilter.get_window(WindowOperations.BLACKMAN_HARRIS.value, window_len)

        print(window_data)


if __name__ == "__main__":
    main()
