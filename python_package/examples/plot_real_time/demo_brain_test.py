import sys
from PyQt5 import QtCore, QtWidgets
import matplotlib
matplotlib.use('Qt5Agg')
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5 import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure
import numpy as np
import brainflow
from brainflow.board_shim import BoardShim, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations
from PyQt5.QtCore import QTimer
from scipy.signal import butter, filtfilt


class EEGDataVisualizer(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        # 初始化self.fig和self.ax，确保在initUI方法使用之前已经存在
        self.fig = Figure(figsize=(200, 200))
        self.ax = self.fig.add_subplot(111)

        self.timer_stopped = False
        self.timer = None
        # 新增buffer_index属性初始化，初始为0
        self.buffer_index = 0
        # 新增data_buffer属性初始化，初始为None
        self.data_buffer = None

        

        # 用于记录坐标轴是否显示的状态变量，初始化为True（显示坐标轴）
        # self.show_axis = True
        # 用于标记是否暂停数据采集和图形更新，初始化为False（未暂停）
        self.paused = False
        # 用于存储暂停时的数据缓冲区内容备份，初始为空
        self.paused_data_buffer = None
        # 用于存储暂停时的缓冲区索引备份，初始为0
        self.paused_buffer_index = 0
        # 存储通道复选框的列表
        self.channel_checkboxes = []
        # 滤波器类型下拉框
        # 滤波器复选框字典，存储每种滤波器的复选框和相关参数输入框等控件
        self.filter_checkboxes = {
            "低通": {"checkbox": None, "cutoff_edit": QtWidgets.QLineEdit()},
            "高通": {"checkbox": None, "cutoff_edit": QtWidgets.QLineEdit()},
            "带通": {"checkbox": None, "low_cutoff_edit": QtWidgets.QLineEdit(), "high_cutoff_edit": QtWidgets.QLineEdit()},
            "带阻": {"checkbox": None, "low_cutoff_edit": QtWidgets.QLineEdit(), "high_cutoff_edit": QtWidgets.QLineEdit()}
        }
        # 低通滤波器默认截止频率
        self.lowpass_default_cutoff = 80.0
        # 高通滤波器默认截止频率
        self.highpass_default_cutoff = 0.5
        # 带通滤波器默认低频截止频率
        self.bandpass_default_low_cutoff = 0.5
        # 带通滤波器默认高频截止频率
        self.bandpass_default_high_cutoff = 45.0
        # 带阻滤波器默认低频截止频率
        self.bandstop_default_low_cutoff = 50.0
        # 带阻滤波器默认高频截止频率
        self.bandstop_default_high_cutoff = 60.0
        self.initUI()

    def initUI(self):
        # 创建主布局
        main_layout = QtWidgets.QHBoxLayout()
        

        # 创建左边布局，用于放置设备连接、采集控制等按钮以及通道选择区域
        left_layout = QtWidgets.QVBoxLayout()

        # MAC 地址输入框
        mac_layout = QtWidgets.QHBoxLayout()
        self.mac_label = QtWidgets.QLabel('MAC 地址:')
        self.mac_edit = QtWidgets.QLineEdit()
        mac_layout.addWidget(self.mac_label)
        mac_layout.addWidget(self.mac_edit)
        left_layout.addLayout(mac_layout)
        # left_layout.addWidget(self.mac_label)
        # left_layout.addWidget(self.mac_edit)

        # board_id 输入框
        id_layout = QtWidgets.QHBoxLayout()
        self.board_id_label = QtWidgets.QLabel('Board ID:')
        self.board_id_edit = QtWidgets.QLineEdit()
        id_layout.addWidget(self.board_id_label)
        id_layout.addWidget(self.board_id_edit)
        left_layout.addLayout(id_layout)
        # left_layout.addWidget(self.board_id_label)
        # left_layout.addWidget(self.board_id_edit)

        # 创建连接设备按钮
        self.connect_button = QtWidgets.QPushButton('连接脑电设备')
        self.connect_button.clicked.connect(self.connect_device)
        left_layout.addWidget(self.connect_button)

        # 创建开始采集按钮（初始不可用，连接设备后可用）
        self.start_button = QtWidgets.QPushButton('开始实时采集脑电数据')
        self.start_button.setEnabled(False)
        self.start_button.clicked.connect(self.start_real_time_collection)
        left_layout.addWidget(self.start_button)

        # 创建暂停按钮（初始不可用，开始采集后可用）
        op_layout = QtWidgets.QHBoxLayout()
        self.pause_button = QtWidgets.QPushButton('暂停')
        self.pause_button.setEnabled(False)
        self.pause_button.clicked.connect(self.pause_real_time_collection)
        # left_layout.addWidget(self.pause_button)
        op_layout.addWidget(self.pause_button)

        # 创建恢复按钮（初始不可用，暂停后可用）
        self.resume_button = QtWidgets.QPushButton('恢复')
        self.resume_button.setEnabled(False)
        self.resume_button.clicked.connect(self.resume_real_time_collection)
        # left_layout.addWidget(self.resume_button)
        op_layout.addWidget(self.resume_button)
        
        # 停止
        self.stop_button = QtWidgets.QPushButton('停止')
        self.stop_button.setEnabled(True)
        self.stop_button.clicked.connect(self.stop_real_time_collection)
        op_layout.addWidget(self.stop_button)
        
        
        left_layout.addLayout(op_layout)
       

        # # 创建显示/隐藏坐标轴按钮
        # self.toggle_axis_button = QtWidgets.QPushButton('显示坐标轴')
        # self.toggle_axis_button.clicked.connect(self.toggle_axis)
        # left_layout.addWidget(self.toggle_axis_button)

        # 创建滤波器复选框及相关输入框布局
        for filter_type in self.filter_checkboxes:
            checkbox = QtWidgets.QCheckBox(filter_type)
            checkbox.stateChanged.connect(self.apply_filter)
            self.filter_checkboxes[filter_type]["checkbox"] = checkbox
            left_layout.addWidget(checkbox)
            if filter_type == "低通":
                cutoff_label = QtWidgets.QLabel('截止频率(Hz):')
                cutoff_edit = self.filter_checkboxes[filter_type]["cutoff_edit"]
                cutoff_edit.setText(str(self.lowpass_default_cutoff))
                cutoff_layout = QtWidgets.QHBoxLayout()
                cutoff_layout.addWidget(cutoff_label)
                cutoff_layout.addWidget(cutoff_edit)
                left_layout.addLayout(cutoff_layout)
            elif filter_type == "高通":
                cutoff_label = QtWidgets.QLabel('截止频率(Hz):')
                cutoff_edit = self.filter_checkboxes[filter_type]["cutoff_edit"]
                cutoff_edit.setText(str(self.highpass_default_cutoff))
                cutoff_layout = QtWidgets.QHBoxLayout()
                cutoff_layout.addWidget(cutoff_label)
                cutoff_layout.addWidget(cutoff_edit)
                left_layout.addLayout(cutoff_layout)
            elif filter_type == "带通":
                low_cutoff_label = QtWidgets.QLabel('低频截止频率(Hz):')
                low_cutoff_edit = self.filter_checkboxes[filter_type]["low_cutoff_edit"]
                low_cutoff_edit.setText(str(self.bandpass_default_low_cutoff))
                high_cutoff_label = QtWidgets.QLabel('高频截止频率(Hz):')
                high_cutoff_edit = self.filter_checkboxes[filter_type]["high_cutoff_edit"]
                high_cutoff_edit.setText(str(self.bandpass_default_high_cutoff))
                low_cutoff_layout = QtWidgets.QHBoxLayout()
                low_cutoff_layout.addWidget(low_cutoff_label)
                low_cutoff_layout.addWidget(low_cutoff_edit)
                high_cutoff_layout = QtWidgets.QHBoxLayout()
                high_cutoff_layout.addWidget(high_cutoff_label)
                high_cutoff_layout.addWidget(high_cutoff_edit)
                bandpass_cutoff_layout = QtWidgets.QVBoxLayout()
                bandpass_cutoff_layout.addLayout(low_cutoff_layout)
                bandpass_cutoff_layout.addLayout(high_cutoff_layout)
                left_layout.addLayout(bandpass_cutoff_layout)
            elif filter_type == "带阻":
                low_cutoff_label = QtWidgets.QLabel('低频截止频率(Hz):')
                low_cutoff_edit = self.filter_checkboxes[filter_type]["low_cutoff_edit"]
                low_cutoff_edit.setText(str(self.bandstop_default_low_cutoff))
                high_cutoff_label = QtWidgets.QLabel('高频截止频率(Hz):')
                high_cutoff_edit = self.filter_checkboxes[filter_type]["high_cutoff_edit"]
                high_cutoff_edit.setText(str(self.bandstop_default_high_cutoff))
                low_cutoff_layout = QtWidgets.QHBoxLayout()
                low_cutoff_layout.addWidget(low_cutoff_label)
                low_cutoff_layout.addWidget(low_cutoff_edit)
                high_cutoff_layout = QtWidgets.QHBoxLayout()
                high_cutoff_layout.addWidget(high_cutoff_label)
                high_cutoff_layout.addWidget(high_cutoff_edit)
                bandstop_cutoff_layout = QtWidgets.QVBoxLayout()
                bandstop_cutoff_layout.addLayout(low_cutoff_layout)
                bandstop_cutoff_layout.addLayout(high_cutoff_layout)
                left_layout.addLayout(bandstop_cutoff_layout)
                
         # 创建用于显示统计指标的标签
        # self.mean_label = QtWidgets.QLabel('均值: ')
        # self.std_label = QtWidgets.QLabel('标准差: ')
        # left_layout.addWidget(self.mean_label,alignment=QtCore.Qt.AlignTop)
        # left_layout.addWidget(self.std_label,alignment=QtCore.Qt.AlignTop)

        # 创建通道选择区域的布局，先初始化为垂直布局，后续根据通道数量可能会改变
        self.channel_layout = QtWidgets.QVBoxLayout()
        # left_layout.addLayout(self.channel_layout)

        # 创建右边布局，用于放置图形显示区域
        right_layout = QtWidgets.QVBoxLayout()

        # 创建matplotlib的FigureCanvas并添加到布局
        self.canvas = FigureCanvas(self.fig)
        # right_layout.addWidget(self.canvas)

        # 创建NavigationToolbar（导航工具栏，提供放大、拖动、复位等功能）并添加到布局
        self.toolbar = NavigationToolbar(self.canvas, self)
        right_layout.addWidget(self.toolbar,alignment=QtCore.Qt.AlignCenter)
        right_layout.addWidget(self.canvas)
        right_layout.addLayout(self.channel_layout)

        # 将左右布局添加到主布局，并设置拉伸因子实现 3:7 的占比
        main_layout.addLayout(left_layout, 1)

        # 创建分割线
        # separator = QtWidgets.QFrame()
        # separator.setFrameShape(QtWidgets.QFrame.VLine)
        # separator.setFrameShadow(QtWidgets.QFrame.Sunken)
        # # 设置样式表，添加投影等效果来增强立体感
        # separator.setStyleSheet("""
        #     border: 1px solid gray;
        #     background-color: white;
        #     box-shadow: 2px 0px 5px rgba(0, 0, 0, 0.1);  /* 水平方向阴影偏移、垂直方向阴影偏移、模糊半径、阴影颜色 */
        # """)
        # main_layout.addWidget(separator)
        main_layout.addLayout(right_layout, 9)

        # 设置图形在布局中的最小尺寸，确保有足够空间显示坐标轴等元素
        main_layout.setContentsMargins(10, 10, 10, 10)

        self.setLayout(main_layout)

        self.setWindowTitle('脑电数据演示界面')
        self.resize(1600, 900) 
        self.show()

        # 初始化设备相关变量
        self.params = None
        self.board_shim = None
        self.lines = []
        self.eeg_channels = []
        
    # def closeEvent(self, event):
    #     if self.board_shim is not None:
    #         self.board_shim.stop_stream()
    #         self.board_shim.release_session()
    #     return super().closeEvent(event)

    def connect_device(self):
        # 获取用户输入的 MAC 地址和 board_id
        mac_address = self.mac_edit.text()
        board_id_text = self.board_id_edit.text()

        try:
            # 设置脑电设备相关参数
            self.board_id = int(board_id_text)
            self.params = brainflow.BrainFlowInputParams()
            self.params.timeout = 40
            if mac_address:
                self.params.mac_address = mac_address
            self.board_shim = BoardShim(self.board_id, self.params)
            self.board_shim.prepare_session()
            # 获取脑电通道列表
            self.eeg_channels = BoardShim.get_eeg_channels(self.board_id)
            # 启用开始采集按钮
            self.start_button.setEnabled(True)
            # 弹出连接成功提示框
            QtWidgets.QMessageBox.information(self, "连接成功", "脑电设备连接成功！")

            # 根据通道数量判断使用何种布局来放置通道复选框
            if len(self.eeg_channels) <= 6:
                self.create_channel_checkboxes_vertical()
            else:
                self.create_channel_checkboxes_grid()
        except ValueError as ve:
            QtWidgets.QMessageBox.critical(self, "连接失败", f"输入的Board ID格式错误，请输入整数类型的Board ID。错误信息：{str(ve)}")
        except brainflow.BrainFlowError as bfe:
            QtWidgets.QMessageBox.critical(self, "连接失败", f"脑电设备连接出现错误，可能原因包括设备未开启、MAC地址错误或者驱动问题等。错误信息：{str(bfe)}")
        except Exception as e:
            QtWidgets.QMessageBox.critical(self, "连接失败", f"未知错误导致设备连接失败，请检查相关配置和设备状态。错误信息：{str(e)}")

    def create_channel_checkboxes_vertical(self):
        for channel in self.eeg_channels:
            checkbox = QtWidgets.QCheckBox(f'Channel {channel}')
            checkbox.setChecked(True)  # 默认勾选所有通道
            checkbox.stateChanged.connect(self.update_channel_visibility)
            self.channel_checkboxes.append(checkbox)
            self.channel_layout.addWidget(checkbox)

    def create_channel_checkboxes_grid(self):
        grid_layout = QtWidgets.QGridLayout()
        row = 0
        col = 0
        max_columns = 4  # 最大列数设置为3，可根据需要调整
        for channel in self.eeg_channels:
            checkbox = QtWidgets.QCheckBox(f'Channel {channel}')
            checkbox.setChecked(True)  # 默认勾选所有通道
            checkbox.stateChanged.connect(self.update_channel_visibility)
            self.channel_checkboxes.append(checkbox)
            grid_layout.addWidget(checkbox, row, col)
            col += 1
            if col >= max_columns:
                col = 0
                row += 1
        self.channel_layout.addLayout(grid_layout)

    def start_real_time_collection(self):
        try:
            # 准备会话并开始数据采集
            self.board_shim.start_stream()

            # 初始化数据缓冲区
            buffer_size = 1000  # 可根据需要调整缓冲区大小
            if len(self.eeg_channels) > 0:
                self.data_buffer = np.zeros((len(self.eeg_channels), buffer_size))
            else:
                raise ValueError("脑电通道数量获取异常，无法初始化数据缓冲区")
            # 初始化buffer_index，这里重新赋值为0，与__init__中的初始化保持一致
            self.buffer_index = 0

            # 启用暂停按钮，禁用开始按钮
            self.pause_button.setEnabled(True)
            self.start_button.setEnabled(False)

            # 开始实时更新图形
            self.timer = self.startTimer(100)  # 每 100 毫秒更新一次
        except brainflow.BrainFlowError as bfe:
            QtWidgets.QMessageBox.critical(self, "数据采集启动失败", f"准备会话或启动数据流时出现错误，请检查设备连接等情况。错误信息：{str(bfe)}")
        except ValueError as ve:
            QtWidgets.QMessageBox.critical(self, "数据缓冲区初始化失败", str(ve))
        except Exception as e:
            QtWidgets.QMessageBox.critical(self, "未知错误", f"启动实时数据采集时出现未知错误，请检查相关配置和代码逻辑。错误信息：{str(e)}")
        
    def timerEvent(self, event):
        if not self.paused and not self.timer_stopped:  # 增加判断定时器是否停止的条件
            # 获取一定数量的新数据点
            new_data = self.board_shim.get_board_data(100)  # 获取100个数据点，可调整
            new_data_channels = None
            if new_data.shape[1] == 0:
                return
            new_data_channels = new_data[self.eeg_channels, :]
            if new_data_channels is not None:
                print("new_data_channels shape:", new_data_channels.shape)
                buffer_size = self.data_buffer.shape[1]
                # 若缓冲区剩余空间不足，重新分配更大的缓冲区空间（示例一种简单的扩容策略，可按需优化）
                if self.buffer_index + new_data_channels.shape[1] > buffer_size:
                    new_buffer_size = max(buffer_size * 2, self.buffer_index + new_data_channels.shape[1])
                    new_data_buffer = np.zeros((len(self.eeg_channels), new_buffer_size))
                    new_data_buffer[:, :self.buffer_index] = self.data_buffer[:, :self.buffer_index]
                    self.data_buffer = new_data_buffer
                # 将新数据添加到缓冲区
                self.data_buffer[:, self.buffer_index:self.buffer_index + new_data_channels.shape[1]] = new_data_channels
                # 更新buffer_index
                self.buffer_index += new_data_channels.shape[1]
                # 计算采样率
                sampling_rate = BoardShim.get_sampling_rate(self.board_id)
                # 确保时间轴起始索引不小于0
                start_index = max(self.buffer_index - new_data_channels.shape[1], 0)
                time_axis = np.arange(start_index, self.buffer_index) / sampling_rate
                # 清除当前坐标轴内容，避免数据叠加显示混乱
                self.ax.clear()
                 # 降噪处理部分，这里以巴特沃斯低通滤波器为例，你可以根据实际需求调整滤波器类型、参数等
                # 定义滤波器参数，截止频率、滤波器阶数等，可根据实际脑电信号特性调整
                cutoff_frequency = 80.0  # 截止频率，单位Hz，示例值，可修改
                filter_order = 4  # 滤波器阶数，示例值，可修改
                nyquist_frequency = 0.5 * sampling_rate
                normalized_cutoff = cutoff_frequency / nyquist_frequency
                b, a = butter(filter_order, normalized_cutoff, btype='low', analog=False)

                # 对每个通道的数据进行滤波（降噪）处理
                for channel in range(len(self.eeg_channels)):
                    data = self.data_buffer[channel, :self.buffer_index]
                    filtered_data = filtfilt(b, a, data)
                    self.data_buffer[channel, :self.buffer_index] = filtered_data
                
                # 更新时域波形图，根据通道勾选状态绘制相应通道的波形
                for channel in range(len(self.eeg_channels)):
                    if self.channel_checkboxes[channel].isChecked():
                        valid_length = min(len(time_axis), self.buffer_index)
                        self.ax.plot(time_axis[:valid_length], self.data_buffer[channel, :valid_length], label=f'Channel {self.eeg_channels[channel]}')
                # 根据坐标轴显示状态设置坐标轴相关属性显示或隐藏，并添加调试相关代码查看图形尺寸等信息
                self.ax.set_xlabel('Time (s)')
                self.ax.set_ylabel('Amplitude (uV)')
                self.ax.set_title('EEG Time Domain Waveforms (Real-time)')
                self.ax.legend(loc='upper right')
                self.fig.canvas.draw_idle()  # 使用draw_idle以更好地处理重绘，避免阻塞UI线程
                self.update()  # 强制重绘整个窗口，确保窗口能及时将图形更新内容显示出来

                # # 计算并更新时域统计指标（均值、标准差），并对结果保留两位小数
                # mean_values = np.mean(self.data_buffer[:, :self.buffer_index], axis=1)
                # std_values = np.std(self.data_buffer[:, :self.buffer_index], axis=1)
                # # 将mean_values中的每个元素格式化为保留两位小数的字符串形式
                # formatted_mean_values = ["{:.2f}".format(x) for x in mean_values]
                # # 将std_values中的每个元素格式化为保留两位小数的字符串形式
                # formatted_std_values = ["{:.2f}".format(x) for x in std_values]
                # 更新均值标签文本内容
                # self.mean_label.setText(f'均值: {", ".join(formatted_mean_values)}')
                # # 更新标准差标签文本内容，修正了之前多余括号的语法错误
                # self.std_label.setText(f'标准差: {", ".join(formatted_std_values)}')

    def pause_real_time_collection(self):
        # 设置暂停状态为True
        self.paused = True
        # 备份当前的数据缓冲区内容和缓冲区索引
        self.paused_data_buffer = self.data_buffer.copy()
        self.paused_buffer_index = self.buffer_index
        # 禁用暂停按钮，启用恢复按钮
        self.pause_button.setEnabled(False)
        self.resume_button.setEnabled(True)

    def resume_real_time_collection(self):
        # 设置暂停状态为False
        self.paused = False
        # 恢复数据缓冲区内容和缓冲区索引
        self.data_buffer = self.paused_data_buffer
        self.buffer_index = self.paused_buffer_index
        # 启用暂停按钮，禁用恢复按钮
        self.pause_button.setEnabled(True)
        self.resume_button.setEnabled(False)

        # 重新启动定时器（如果之前定时器因为暂停被停止了，这里重新启动）
        
        if not isinstance(self.timer, QTimer):
            self.timer = self.startTimer(100)
        elif not self.timer.isActive():
            self.timer.start(100)
            
    def stop_real_time_collection(self):
        if self.timer is not None and isinstance(self.timer, QTimer) and self.timer.isActive():
            self.timer.stop()
            self.timer_stopped = True  # 设置标志位，表示定时器已停止
        if self.board_shim is not None:
            try:
                self.board_shim.stop_stream()
                if self.timer_stopped:  # 只有定时器停止了才释放会话资源
                    self.board_shim.release_session()
                    print("已成功停止数据流并释放会话资源")
            except Exception as e:
                print(f"停止数据流或释放会话资源时出现错误: {e}")

        # 以下是新增的清理相关资源和重置界面、变量的代码部分

        # 清空数据缓冲区
        self.data_buffer = None
        self.buffer_index = 0

        # 重置暂停相关的变量和数据备份
        self.paused = False
        self.paused_data_buffer = None
        self.paused_buffer_index = None

        # 禁用相关按钮（除了连接设备按钮外，其他采集控制按钮都设为不可用）
        self.start_button.setEnabled(False)
        self.pause_button.setEnabled(False)
        self.resume_button.setEnabled(False)
        self.stop_button.setEnabled(False)

        # 清空通道复选框列表，并从布局中移除对应的复选框（假设通道复选框所在布局是self.channel_layout，需根据实际情况调整）
        for checkbox in self.channel_checkboxes:
            checkbox.deleteLater()
        self.channel_checkboxes.clear()

        # # 清空均值、标准差等统计指标标签显示内容
        # self.mean_label.setText("均值: ")
        # self.std_label.setText("标准差: ")

        # 清空图形绘制相关内容（假设图形的坐标轴是self.ax，画布是self.canvas，需根据实际情况调整）
        self.ax.clear()
        self.canvas.draw_idle()
            

    # def toggle_axis(self):
    #     print("Before toggle, show_axis:", self.show_axis)
    #     self.show_axis = not self.show_axis
    #     print("After toggle, show_axis:", self.show_axis)
    #     if self.show_axis:
    #         self.toggle_axis_button.setText('显示坐标轴')
    #         self.ax.axis('on')
    #         self.ax.set_xlabel('Time (s)')
    #         self.ax.set_ylabel('Amplitude (uV)')
    #         self.ax.set_title('EEG Time Domain Waveforms (Real-time)')
    #         self.ax.legend(loc='upper right')
    #     else:
    #         self.toggle_axis_button.setText('隐藏坐标轴')
    #         self.ax.axis('off')

    #     self.fig.canvas.draw_idle()
    #     self.update()  # 强制重绘整个窗口，确保窗口能及时显示图形更新内容

    def update_channel_visibility(self):
        # 当通道勾选状态改变时，更新图形显示
        self.ax.clear()
        sampling_rate = BoardShim.get_sampling_rate(self.board_id)
        if self.buffer_index > 0:
            time_axis = np.arange(0, self.buffer_index) / sampling_rate
            for channel in range(len(self.eeg_channels)):
                if self.channel_checkboxes[channel].isChecked():
                    valid_length = min(len(time_axis), self.buffer_index)
                    self.ax.plot(time_axis[:valid_length], self.data_buffer[channel, :valid_length], label=f'Channel {self.eeg_channels[channel]}')
        else:
            time_axis = np.array([])  # 初始化一个空数组，避免为None
            for channel in range(len(self.eeg_channels)):
                if self.channel_checkboxes[channel].isChecked():
                    self.ax.plot(time_axis, self.data_buffer[channel, :self.buffer_index], label=f'Channel {self.eeg_channels[channel]}')

        self.ax.legend(loc='upper right')
        self.fig.canvas.draw_idle()
        self.update()
        
    def update_graph(self):
        sampling_rate = BoardShim.get_sampling_rate(self.board_id)
        self.ax.clear()

        # 根据通道勾选状态绘制相应通道的波形
        time_axis = np.arange(0, self.buffer_index) / sampling_rate
        for channel in range(len(self.eeg_channels)):
            if self.channel_checkboxes[channel].isChecked():
                valid_length = min(len(time_axis), self.buffer_index)
                self.ax.plot(time_axis[:valid_length], self.data_buffer[channel, :valid_length], label=f'Channel {self.eeg_channels[channel]}')

        # 根据坐标轴显示状态设置坐标轴相关属性显示或隐藏，并添加调试相关代码查看图形尺寸等信息
        print("In update_graph, show_axis:", self.show_axis)
        if self.show_axis:
            self.ax.set_xlabel('Time (s)')
            self.ax.set_ylabel('Amplitude (uV)')
            self.ax.set_title('EEG Time Domain Waveforms (Real-time)')
            self.ax.legend(loc='upper right')
        else:
            self.ax.axis('off')

        self.fig.canvas.draw_idle()
        self.update()

    def apply_filter(self):
        sampling_rate = BoardShim.get_sampling_rate(self.board_id)
        # 遍历每种滤波器的复选框及相关参数输入框状态，进行滤波操作判断和执行
        for filter_type in self.filter_checkboxes:
            checkbox = self.filter_checkboxes[filter_type]["checkbox"]
            if checkbox.isChecked():
                if filter_type == "低通":
                    cutoff_frequency = float(self.filter_checkboxes[filter_type]["cutoff_edit"].text())
                    for channel in range(self.data_buffer.shape[0]):
                        channel_data = self.data_buffer[channel, :].flatten()
                        DataFilter.perform_lowpass(channel_data, sampling_rate, cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
                elif filter_type == "高通":
                    cutoff_frequency = float(self.filter_checkboxes[filter_type]["cutoff_edit"].text())
                    for channel in range(self.data_buffer.shape[0]):
                        channel_data = self.data_buffer[channel, :].flatten()
                        DataFilter.perform_highpass(channel_data, sampling_rate, cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
                elif filter_type == "带通":
                    low_cutoff_frequency = float(self.filter_checkboxes[filter_type]["low_cutoff_edit"].text())
                    high_cutoff_frequency = float(self.filter_checkboxes[filter_type]["high_cutoff_edit"].text())
                    for channel in range(self.data_buffer.shape[0]):
                        channel_data = self.data_buffer[channel, :].flatten()
                        DataFilter.perform_bandpass(channel_data, sampling_rate, low_cutoff_frequency, high_cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
                elif filter_type == "带阻":
                    low_cutoff_frequency = float(self.filter_checkboxes[filter_type]["low_cutoff_edit"].text())
                    high_cutoff_frequency = float(self.filter_checkboxes[filter_type]["high_cutoff_edit"].text())
                    for channel in range(self.data_buffer.shape[0]):
                        channel_data = self.data_buffer[channel, :].flatten()
                        DataFilter.perform_bandstop(channel_data, sampling_rate, low_cutoff_frequency, high_cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
        # 滤波后更新图形显示（可根据实际情况调用相应更新图形的方法，此处假设调用update_graph方法，你需要按实际已有逻辑调整）
        self.update_graph()

    def apply_filter_to_data(self, sampling_rate, filter_type):
        if filter_type == "低通":
            cutoff_frequency = float(self.filter_checkboxes[filter_type]["cutoff_edit"].text())
            for channel in range(self.data_buffer.shape[0]):
                # 提取单个通道的数据，并展平为一维数组
                channel_data = self.data_buffer[channel, :].flatten()
                DataFilter.perform_lowpass(channel_data, sampling_rate, cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
        elif filter_type == "高通":
            cutoff_frequency = float(self.filter_checkboxes[filter_type]["cutoff_edit"].text())
            for channel in range(self.data_buffer.shape[0]):
                channel_data = self.data_buffer[channel, :].flatten()
                DataFilter.perform_highpass(channel_data, sampling_rate, cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
        elif filter_type == "带通":
            low_cutoff_frequency = float(self.filter_checkboxes[filter_type]["low_cutoff_edit"].text())
            high_cutoff_frequency = float(self.filter_checkboxes[filter_type]["high_cutoff_edit"].text())
            for channel in range(self.data_buffer.shape[0]):
                channel_data = self.data_buffer[channel, :].flatten()
                DataFilter.perform_bandpass(channel_data, sampling_rate, low_cutoff_frequency, high_cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
        elif filter_type == "带阻":
            low_cutoff_frequency = float(self.filter_checkboxes[filter_type]["low_cutoff_edit"].text())
            high_cutoff_frequency = float(self.filter_checkboxes[filter_type]["high_cutoff_edit"].text())
            for channel in range(self.data_buffer.shape[0]):
                channel_data = self.data_buffer[channel, :].flatten()
                DataFilter.perform_bandstop(channel_data, sampling_rate, low_cutoff_frequency, high_cutoff_frequency, 2, FilterTypes.BUTTERWORTH.value, 0)
if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    ex = EEGDataVisualizer()
    sys.exit(app.exec_())