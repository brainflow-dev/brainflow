import argparse
import logging

import pyqtgraph as pg
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, WindowOperations, DetrendOperations
from pyqtgraph.Qt import QtGui, QtCore

#python d:\workspace\brainflow\python_package\examples\plot_real_time\plot_real_time.py --board-id=57 --mac-address=C4:64:E3:D8:E6:C6

#python d:\workspace\brainflow\python_package\examples\plot_real_time\plot_real_time.py --board-id=57 --mac-address=C4:64:E3:D8:E4:8D

#python d:\workspace\brainflow\python_package\examples\plot_real_time\plot_real_time.py --board-id=58 --mac-address=60:77:71:74:E6:B7


class Graph:
    def __init__(self, board_shims):
        pg.setConfigOption('background', 'w')
        pg.setConfigOption('foreground', 'k')

        self.board_shims = board_shims
        self.multi_exg_channels = []
        self.total_channels = 0
        for board_shim in board_shims:
            self.board_id = board_shim.get_board_id()
            self.multi_exg_channels.append(BoardShim.get_exg_channels(self.board_id))
            self.total_channels += len(BoardShim.get_exg_channels(self.board_id))
            self.sampling_rate = BoardShim.get_sampling_rate(self.board_id)

        self.update_speed_ms = 50
        self.window_size = 4
        self.num_points = self.window_size * self.sampling_rate

        self.app = QtGui.QApplication([])
        self.win = pg.GraphicsWindow(title='BrainFlow Plot', size=(800, 600))

        self._init_pens()
        self._init_timeseries()
        self._init_psd()
        self._init_band_plot()

        timer = QtCore.QTimer()
        timer.timeout.connect(self.update)
        timer.start(self.update_speed_ms)
        QtGui.QApplication.instance().exec_()

    def _init_pens(self):
        self.pens = list()
        self.brushes = list()
        colors = ['#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F','#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F','#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F','#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F','#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F','#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F','#A54E4E', '#A473B6', '#5B45A4', '#2079D2', '#32B798', '#2FA537', '#9DA52F', '#A57E2F', '#A53B2F']
        for i in range(len(colors)):
            pen = pg.mkPen({'color': colors[i], 'width': 2})
            self.pens.append(pen)
            brush = pg.mkBrush(colors[i])
            self.brushes.append(brush)

    def _init_timeseries(self):
        self.plots = list()
        self.curves = list()
        for i in range(self.total_channels):
            p = self.win.addPlot(row=i, col=0)
            p.showAxis('left', False)
            p.setMenuEnabled('left', False)
            p.showAxis('bottom', False)
            p.setMenuEnabled('bottom', False)
            if i == 0:
                p.setTitle('TimeSeries Plot')
            self.plots.append(p)
            curve = p.plot(pen=self.pens[i % len(self.pens)])
            # curve.setDownsampling(auto=True, method='mean', ds=3)
            self.curves.append(curve)

    def _init_psd(self):
        self.psd_plot = self.win.addPlot(row=0, col=1, rowspan=self.total_channels // 2)
        self.psd_plot.showAxis('left', False)
        self.psd_plot.setMenuEnabled('left', False)
        self.psd_plot.setTitle('PSD Plot')
        self.psd_plot.setLogMode(False, True)
        self.psd_curves = list()
        self.psd_size = DataFilter.get_nearest_power_of_two(self.sampling_rate)
        for i in range(self.total_channels):
            psd_curve = self.psd_plot.plot(pen=self.pens[i % len(self.pens)])
            psd_curve.setDownsampling(auto=True, method='mean', ds=3)
            self.psd_curves.append(psd_curve)

    def _init_band_plot(self):
        self.band_plot = self.win.addPlot(row=self.total_channels // 2, col=1, rowspan=self.total_channels // 2)
        self.band_plot.showAxis('left', False)
        self.band_plot.setMenuEnabled('left', False)
        self.band_plot.showAxis('bottom', False)
        self.band_plot.setMenuEnabled('bottom', False)
        self.band_plot.setTitle('BandPower Plot')
        y = [0, 0, 0, 0, 0]
        x = [1, 2, 3, 4, 5]
        self.band_bar = pg.BarGraphItem(x=x, height=y, width=0.8, pen=self.pens[0], brush=self.brushes[0])
        self.band_plot.addItem(self.band_bar)

    def update(self):
        baseIdx = 0
        for idx in range(len(self.board_shims)):
            data = self.board_shims[idx].get_current_board_data(self.num_points)
            avg_bands = [0, 0, 0, 0, 0]
            for count, channel in enumerate(self.multi_exg_channels[idx]):
                # plot timeseries
                DataFilter.detrend(data[channel], DetrendOperations.CONSTANT.value)
                DataFilter.perform_bandpass(data[channel], self.sampling_rate, 3.0, 45.0, 2,
                                            FilterTypes.BUTTERWORTH_ZERO_PHASE, 0)
                DataFilter.perform_bandstop(data[channel], self.sampling_rate, 48.0, 52.0, 2,
                                            FilterTypes.BUTTERWORTH_ZERO_PHASE, 0)
                DataFilter.perform_bandstop(data[channel], self.sampling_rate, 58.0, 62.0, 2,
                                            FilterTypes.BUTTERWORTH_ZERO_PHASE, 0)
                self.curves[baseIdx + count].setData(data[channel].tolist())
                
                if data.shape[1] > self.psd_size:
                    # plot psd
                    psd_data = DataFilter.get_psd_welch(data[channel], self.psd_size, self.psd_size // 2,
                                                        self.sampling_rate,
                                                        WindowOperations.BLACKMAN_HARRIS.value)
                    lim = min(70, len(psd_data[0]))
                    self.psd_curves[baseIdx + count].setData(psd_data[1][0:lim].tolist(), psd_data[0][0:lim].tolist())
                    # plot bands
                    avg_bands[0] = avg_bands[0] + DataFilter.get_band_power(psd_data, 2.0, 4.0)
                    avg_bands[1] = avg_bands[1] + DataFilter.get_band_power(psd_data, 4.0, 8.0)
                    avg_bands[2] = avg_bands[2] + DataFilter.get_band_power(psd_data, 8.0, 13.0)
                    avg_bands[3] = avg_bands[3] + DataFilter.get_band_power(psd_data, 13.0, 30.0)
                    avg_bands[4] = avg_bands[4] + DataFilter.get_band_power(psd_data, 30.0, 50.0)

            baseIdx += len(self.multi_exg_channels[idx])

        avg_bands = [int(x * 100 / self.total_channels) for x in avg_bands]
        self.band_bar.setOpts(height=avg_bands)

        self.app.processEvents()


def main():
    BoardShim.enable_dev_board_logger()
    logging.basicConfig(level=logging.DEBUG)

    parser = argparse.ArgumentParser()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument('--timeout', type=int, help='timeout for device discovery or connection', required=False,
                        default=0)
    parser.add_argument('--ip-port', type=int, help='ip port', required=False, default=0)
    parser.add_argument('--ip-protocol', type=int, help='ip protocol, check IpProtocolType enum', required=False,
                        default=0)
    parser.add_argument('--ip-address', type=str, help='ip address', required=False, default='')
    parser.add_argument('--serial-port', type=str, help='serial port', required=False, default='')
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    parser.add_argument('--other-info', type=str, help='other info', required=False, default='')
    parser.add_argument('--streamer-params', type=str, help='streamer params', required=False, default='')
    parser.add_argument('--serial-number', type=str, help='serial number', required=False, default='')
    parser.add_argument('--board-id', type=int, help='board id, check docs to get a list of supported boards',
                        required=False, default=BoardIds.SYNTHETIC_BOARD)
    parser.add_argument('--file', type=str, help='file', required=False, default='')
    parser.add_argument('--master-board', type=int, help='master board id for streaming and playback boards',
                        required=False, default=BoardIds.NO_BOARD)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.serial_number = args.serial_number
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol
    params.timeout = args.timeout
    params.file = args.file
    params.master_board = args.master_board

    args.board_id = 57
    params.mac_address = 'C4:64:E3:D8:E6:C6'
    board_shim1 = BoardShim(args.board_id, params)

    args.board_id = 57
    params.mac_address = 'C4:64:E3:D8:E4:8D'
    board_shim2 = BoardShim(args.board_id, params)

    args.board_id = 58
    params.mac_address = '60:77:71:74:E6:B7'
    board_shim3 = BoardShim(args.board_id, params)

    # boards = [board_shim1, board_shim2, board_shim3]
    # boards = [board_shim1, board_shim2]
    # boards = [board_shim1]
    # boards = [board_shim1, board_shim3]
    boards = [board_shim3]

    for board_shim in boards:
        try:
            board_shim.prepare_session()
            board_shim.start_stream(450000, args.streamer_params)
        except BaseException:
            logging.warning('Exception', exc_info=True)
            if board_shim.is_prepared():
                logging.info('Releasing session')
                board_shim.release_session()

    Graph(boards)
    for board_shim in boards:
        if board_shim.is_prepared():
            logging.info('Releasing session')
            board_shim.release_session()

if __name__ == '__main__':
    main()
