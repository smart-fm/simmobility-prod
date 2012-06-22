#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
from gzip import GzipFile
from bz2 import BZ2File

import ui_controller
from session import Session
from static.network import Network
from dynamic.city import City
from animator import Animator
from scene import Graphics_scene

class Controller(QtGui.QDockWidget,
                 ui_controller.Ui_controller):
    def __init__(self, parent, visualizer_view, status_bar):
        super(Controller, self).__init__(parent)
        self.parent = parent
        self.setupUi(self)

        self.visualizer_view = visualizer_view
        self.status_bar = status_bar

    @QtCore.pyqtSignature("int")
    def on_frame_number_spin_box_valueChanged(self, frame_number):
        self.frame_number_slider.setValue(frame_number)
        if frame_number > self.session.warm_up_time:
            self.warm_up_status_label.setText('<span style="color: green;">Warm-up completed</span>')
        else:
            self.warm_up_status_label.setText('<strong style="color: red;">Warming up</strong>')
        self.set_time_display(frame_number)
        self.animator.go_to_frame_number(frame_number)

    @QtCore.pyqtSignature("int")
    def on_frame_number_slider_valueChanged(self, frame_number):
        self.frame_number_spin_box.setValue(frame_number)

    def set_time_display(self, frame_number):
        elapsed = frame_number * self.session.frame_interval
        time = self.start_time.addMSecs(elapsed)

        hour = time.hour()
        if hour >= 12:
            self.am_or_pm_push_button.setChecked(True)
        else:
            self.am_or_pm_push_button.setChecked(False)
            # In 12-hour time, the 0th hour is not spoken; in its place is the 12th hour.
            if hour == 0:
                hour = 12
        self.hour_spin_box.setValue(hour)
        self.minute_spin_box.setValue(time.minute())
        self.second_spin_box.setValue(time.second())
        self.fraction_second_spin_box.setValue(time.msec() / self.session.frame_interval)
        #print "%02d:%02d:%02d.%d %s" % (time.hour(), time.minute(), time.second(),
        #                                time.msec() / self.session.frame_interval,
        #                                "AM" if self.am_or_pm_push_button.isChecked() else "PM")

    @QtCore.pyqtSignature("int")
    def on_hour_spin_box_valueChanged(self, hour):
        self.set_frame_number()

    @QtCore.pyqtSignature("int")
    def on_minute_spin_box_valueChanged(self, hour):
        self.set_frame_number()

    @QtCore.pyqtSignature("int")
    def on_second_spin_box_valueChanged(self, hour):
        self.set_frame_number()

    @QtCore.pyqtSignature("int")
    def on_fraction_second_spin_box_valueChanged(self, hour):
        self.set_frame_number()

    @QtCore.pyqtSignature("bool")
    def on_am_or_pm_push_button_toggled(self, state):
        if state:
            self.am_or_pm_push_button.setText("PM")
        else:
            self.am_or_pm_push_button.setText("AM")
        self.set_frame_number()

    def set_frame_number(self):
        hour = self.hour_spin_box.value()
        minute = self.minute_spin_box.value()
        second = self.second_spin_box.value()
        milli_second = self.fraction_second_spin_box.value() * self.session.frame_interval
        if self.am_or_pm_push_button.isChecked():
            if hour != 12:
                hour += 12
        elif hour == 12:
            hour -= 12

        time = QtCore.QTime(hour, minute, second, milli_second)
        elapsed = self.start_time.msecsTo(time)
        elapsed /= self.session.frame_interval

        if elapsed < 0:
            elapsed = 0
            self.set_time_display(elapsed)
        elif elapsed > self.session.total_run_time:
            elapsed = self.session.total_run_time - 1
            self.set_time_display(elapsed)
        self.frame_number_spin_box.setValue(elapsed)
        #print "setting frame number to %d" % elapsed

    @QtCore.pyqtSignature("int")
    def on_playback_speed_slider_valueChanged(self, speed):
        self.set_playback_speed(speed)

    def set_playback_speed(self, speed):
        if speed == 0:
            self.playback_speed_line_edit.setText(u"1\u00d7")
            self.playback_speed_description_label.setText("real-time")
            self.interval = self.session.frame_interval
        elif speed > 0:
            factor = speed + 1
            self.playback_speed_line_edit.setText(u"%d\u00d7" % factor)
            self.playback_speed_description_label.setText("quick-time")
            self.interval = self.session.frame_interval / factor
        else:
            factor = -speed + 1
            self.playback_speed_line_edit.setText(u"\u00f7%d" % factor)
            self.playback_speed_description_label.setText("slow-motion")
            self.interval = self.session.frame_interval * factor

    @QtCore.pyqtSignature("")
    def on_load_file_push_button_clicked(self):
        file_name = QtGui.QFileDialog.getOpenFileName(self.parent, "Choose file to load", ".")
        if len(file_name):
            self.load_log_file(str(file_name))

    def load_log_file(self, file_name):
        self.scene = Graphics_scene(self)
        self.visualizer_view.setScene(self.scene)

        self.session = Session()
        self.road_network = Network()
        self.animator = Animator(self.scene)
        self.city = City(self.animator)

        if file_name.endswith(".gz"):
            input_file = GzipFile(file_name)
        elif file_name.endswith(".bz2"):
            input_file = BZ2File(file_name)
        else:
            input_file = open(file_name, 'r')
        for line in input_file:
            if self.session.parse(line):
                continue
            if self.road_network.parse(line):
                continue
            if self.city.parse(line):
                continue
        input_file.close()

        self.road_network.resolve()
        self.scene.draw_road_network(self.road_network)

        self.set_ready_to_run(file_name)

    def set_ready_to_run(self, file_name):
        self.log_file_name_line_edit.setText(file_name)
        # Show the agents at frame 0.
        self.animator.go_to_frame_number(0)

        self.start_time = QtCore.QTime(self.session.start_hour, self.session.start_minute,
                                       self.session.start_second)
        last_frame_number = self.session.total_run_time
        self.frame_number_spin_box.setValue(0)
        self.frame_number_spin_box.setMaximum(last_frame_number - 1)
        self.last_frame_number_spin_box.setMaximum(last_frame_number + 1)
        self.last_frame_number_spin_box.setValue(last_frame_number)
        self.warm_up_time_line_edit.setText("%d (frames)" % self.session.warm_up_time)
        self.frame_number_slider.setMaximum(last_frame_number + 1)
        if last_frame_number / self.frame_number_slider.tickInterval() < 20:
            self.frame_number_slider.setTickPosition(QtGui.QSlider.TicksAbove)
        else:
            self.frame_number_slider.setTickPosition(QtGui.QSlider.NoTicks)
        self.set_time_display(0)
        end_time = self.start_time.addMSecs(last_frame_number * self.session.frame_interval)
        self.end_time_line_edit.setTime(end_time)
        self.set_playback_speed(self.playback_speed_slider.value())

        self.frame_number_spin_box.setEnabled(True)
        self.frame_number_slider.setEnabled(True)
        self.hour_spin_box.setEnabled(True)
        self.minute_spin_box.setEnabled(True)
        self.second_spin_box.setEnabled(True)
        self.fraction_second_spin_box.setEnabled(True)
        self.am_or_pm_push_button.setEnabled(True)
        self.playback_speed_slider.setEnabled(True)
        self.stop_or_go_push_button.setEnabled(True)

    @QtCore.pyqtSignature("bool")
    def on_stop_or_go_push_button_toggled(self, state):
        button = self.stop_or_go_push_button
        if button.isChecked():
            button.setText("Stop")
            self.switch_to_running_mode()
        else:
            button.setText("Go")
            self.switch_to_paused_mode()

    def switch_to_running_mode(self):
        self.frame_number_spin_box.setEnabled(False)
        self.frame_number_slider.setEnabled(False)
        self.hour_spin_box.setEnabled(False)
        self.minute_spin_box.setEnabled(False)
        self.second_spin_box.setEnabled(False)
        self.fraction_second_spin_box.setEnabled(False)
        self.am_or_pm_push_button.setEnabled(False)
        self.load_file_push_button.setEnabled(False)
        self.is_running = True
        QtCore.QTimer.singleShot(self.interval, self.run)

    def switch_to_paused_mode(self):
        self.is_running = False
        self.frame_number_spin_box.setEnabled(True)
        self.frame_number_slider.setEnabled(True)
        self.hour_spin_box.setEnabled(True)
        self.minute_spin_box.setEnabled(True)
        self.second_spin_box.setEnabled(True)
        self.fraction_second_spin_box.setEnabled(True)
        self.am_or_pm_push_button.setEnabled(True)
        self.load_file_push_button.setEnabled(True)

    def run(self):
        self.frame_number_spin_box.stepUp()

        if self.is_running:
            QtCore.QTimer.singleShot(self.interval, self.run)
