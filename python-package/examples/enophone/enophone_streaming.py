import argparse
import logging

import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore

import enotools
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


class Graph:
    def __init__(self, board_shim):
        self.board_id = board_shim.get_board_id()
        self.board_shim = board_shim
        self.exg_channels = BoardShim.get_exg_channels(self.board_id)
        self.sampling_rate = BoardShim.get_sampling_rate(self.board_id)
        self.update_speed_ms = 50
        self.window_size = 10
        self.num_points = self.window_size * self.sampling_rate
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

        data = self.board_shim.get_current_board_data(self.num_points)

        if data.shape[1] > 3*self.sampling_rate:
            if self.mains is None:
                self.mains = enotools.detect_mains(data)
            quality = enotools.quality(data)
            data = enotools.referencing(data, mode='mastoid')
            data = enotools.signal_filtering(data,filter_cut=250,bandpass_range=[3,40],bandstop_range=self.mains)

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
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.mac_address = args.mac_address

    try:
        board_shim = BoardShim(BoardIds.ENOPHONE_BOARD, params)
        board_shim.prepare_session()
        board_shim.start_stream(450000, '')
    except BaseException:
        logging.warning('Exception', exc_info=True)
    finally:
        logging.info('End')
        if board_shim.is_prepared():
            logging.info('Releasing session')
            board_shim.release_session()


if __name__ == '__main__':
    main()
