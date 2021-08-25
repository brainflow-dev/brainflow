import argparse
import time
import logging
import random
import numpy as np

import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore

import brainflow
import enotools
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowError
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, WindowFunctions, DetrendOperations


class Graph:
    def __init__(self, board_shim):
        self.board_id = board_shim.get_board_id()
        self.board_shim = board_shim
        self.exg_channels = BoardShim.get_exg_channels(self.board_id)
        self.sampling_rate = BoardShim.get_sampling_rate(self.board_id)
        self.update_speed_ms = 50
        self.window_size = 2500
        self.plot_names = ['Quality A2','Quality A1','Quality C4','Quality C3']
        self.mains = None
        self.app = QtGui.QApplication([])
        self.win = pg.GraphicsWindow(title='Enophone Live Streaming',size=(800, 600))
        self._init_timeseries()

        timer = QtCore.QTimer()
        timer.timeout.connect(self.update)
        timer.start(self.update_speed_ms)
        QtGui.QApplication.instance().exec_()


    def _init_timeseries(self):
        self.plots = list()
        self.curves = list()
        self.legends = list()

        for i in range(len(self.exg_channels)):
            p = self.win.addPlot(row=i,col=0)
            legend = p.addLegend(brush='k')
            p.showAxis('left', False)
            p.setMenuEnabled('left', False)
            p.showAxis('bottom', False)
            p.setMenuEnabled('bottom', False)
            p.setYRange(-100, 100, padding=0)
            if i == 0:
                p.setTitle('Live Enophone Data')
            self.plots.append(p)
            curve = p.plot(name=self.plot_names[i])
            self.curves.append(curve)
            self.legends.append(legend)

    def update(self):

        data = self.board_shim.get_current_board_data(self.window_size)

        if data.shape[1] > 750:
            if self.mains is None:
                self.mains = enotools.detect_mains(data)
            quality = enotools.quality(data)
            data = enotools.referencing(data, mode='mastoid')
            data = enotools.signal_filtering(data,filter_cut=250,bandpass_range=[1,40],bandstop_range=self.mains)

            for count, channel in enumerate(self.exg_channels):
                name = self.plot_names[count] + ': ' + str(quality[count])
                self.curves[count].setData(data[channel].tolist())
                self.legends[count].getLabel(self.curves[count]).setText(name)
                if quality[count] >= 99:
                    self.legends[count].setLabelTextColor('g')
                elif quality[count] >= 95:
                    self.legends[count].setLabelTextColor('y')
                else:
                    self.legends[count].setLabelTextColor('r')

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
                        required=False, default='37')
    parser.add_argument('--file', type=str, help='file', required=False, default='')
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

    try:
        board_shim = BoardShim(args.board_id, params)
        board_shim.prepare_session()
        board_shim.start_stream(450000, args.streamer_params)
        g = Graph(board_shim)
    except BaseException as e:
        logging.warning('Exception', exc_info=True)
    finally:
        logging.info('End')
        if board_shim.is_prepared():
            logging.info('Releasing session')
            board_shim.release_session()


if __name__ == '__main__':
    main()
