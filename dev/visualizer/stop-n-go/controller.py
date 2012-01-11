#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Controller(QtGui.QDockWidget):
    def __init__(self, parent=None):
        super(Controller, self).__init__("Stop and Go", parent)
        self.setFeatures(self.features() & ~QtGui.QDockWidget.DockWidgetClosable)
        self.visualizer = parent

        widget = QtGui.QWidget()
        self.setWidget(widget)
        layout = QtGui.QVBoxLayout()
        widget.setLayout(layout)

        self.add_frame_number_spin_box(layout)
        self.add_simulation_time_spin_box(layout)
        self.add_playback_speed_spin_box(layout)
        self.add_load_file_button(layout)
        self.add_stop_go_button(layout)
        layout.addStretch(1)

        self.interval = 100

    def add_frame_number_spin_box(self, layout):
        spin = QtGui.QSpinBox()
        font = spin.font()
        font.setPointSize(24)
        font.setFixedPitch(True)
        spin.setFont(font)
        spin.setMinimum(0)
        spin.setMaximum(999999)
        self.frame_number_spin_box = spin

        group_box = QtGui.QGroupBox("Frame_number")
        vbox_layout = QtGui.QVBoxLayout()
        vbox_layout.addWidget(spin)
        group_box.setLayout(vbox_layout)

        layout.addWidget(group_box)

    def add_simulation_time_spin_box(self, layout):
        hour_spin = QtGui.QSpinBox()
        hour_spin.setMinimum(0)
        hour_spin.setMaximum(12)
        hour_spin.setFrame(False)
        hour_spin.setButtonSymbols(QtGui.QAbstractSpinBox.NoButtons)

        hour_minute_colon_label = QtGui.QLabel(":")

        minute_spin = QtGui.QSpinBox()
        minute_spin.setMinimum(0)
        minute_spin.setMaximum(59)
        minute_spin.setFrame(False)
        minute_spin.setButtonSymbols(QtGui.QAbstractSpinBox.NoButtons)

        minute_second_colon_label = QtGui.QLabel(":")

        second_spin = QtGui.QSpinBox()
        second_spin.setMinimum(0)
        second_spin.setMaximum(59)
        second_spin.setFrame(False)
        second_spin.setButtonSymbols(QtGui.QAbstractSpinBox.NoButtons)

        second_fraction_second_dot_label = QtGui.QLabel(".")

        fraction_second_spin = QtGui.QSpinBox()
        fraction_second_spin.setMinimum(0)
        fraction_second_spin.setMaximum(9)
        fraction_second_spin.setFrame(False)
        fraction_second_spin.setButtonSymbols(QtGui.QAbstractSpinBox.NoButtons)

        am_or_pm_label = QtGui.QLabel("AM")

        hbox_layout = QtGui.QHBoxLayout()
        hbox_layout.addWidget(hour_spin)
        hbox_layout.addWidget(hour_minute_colon_label)
        hbox_layout.addWidget(minute_spin)
        hbox_layout.addWidget(minute_second_colon_label)
        hbox_layout.addWidget(second_spin)
        hbox_layout.addWidget(second_fraction_second_dot_label)
        hbox_layout.addWidget(fraction_second_spin)
        hbox_layout.addWidget(am_or_pm_label)
        hbox_layout.addStretch(1)

        date_label = QtGui.QLabel("Tue Jan 10 SST 2012")

        group_box = QtGui.QGroupBox("Simulation time")
        vbox_layout = QtGui.QVBoxLayout()
        vbox_layout.addLayout(hbox_layout)
        vbox_layout.addWidget(date_label)
        group_box.setLayout(vbox_layout)

        layout.addWidget(group_box)

    def add_playback_speed_spin_box(self, layout):
        spin = QtGui.QSpinBox()
        spin.setMinimum(-5)
        spin.setMaximum(5)

        group_box = QtGui.QGroupBox("Playback speed")
        vbox_layout = QtGui.QVBoxLayout()
        vbox_layout.addWidget(spin)
        group_box.setLayout(vbox_layout)

        layout.addWidget(group_box)

    def add_load_file_button(self, layout):
        label = QtGui.QLabel("File:")
        line_edit = QtGui.QLineEdit()
        line_edit.setEnabled(False)
        label.setBuddy(line_edit)
        hbox_layout = QtGui.QHBoxLayout()
        hbox_layout.addWidget(label)
        hbox_layout.addWidget(line_edit)
        self.simulator_log_file_line_edit = line_edit

        button = QtGui.QPushButton("Load file")
        self.connect(button, QtCore.SIGNAL("clicked()"), self.choose_file)

        group_box = QtGui.QGroupBox("Simulator Log File")
        vbox_layout = QtGui.QVBoxLayout()
        vbox_layout.addLayout(hbox_layout)
        vbox_layout.addWidget(button)
        group_box.setLayout(vbox_layout)

        layout.addWidget(group_box)

    def choose_file(self):
        file_name = QtGui.QFileDialog.getOpenFileName(self.visualizer, "Choose file to load", ".")
        if len(file_name):
            self.simulator_log_file_line_edit.setText(file_name)
            self.visualizer.load(str(file_name))

    def add_stop_go_button(self, layout):
        button = QtGui.QPushButton("Go")
        layout.addWidget(button)
        button.setCheckable(True)
        button.setChecked(False)
        self.connect(button, QtCore.SIGNAL("toggled(bool)"), self.stop_or_go)

    def stop_or_go(self, state):
        button = self.sender()
        if button.isChecked():
            button.setText("Stop")
            self.frame_number_spin_box.setEnabled(False)
            self.frame_number_spin_box.setReadOnly(True)
            self.is_running = True
            QtCore.QTimer.singleShot(self.interval, self.run)
        else:
            button.setText("Go")
            self.is_running = False
            self.frame_number_spin_box.setEnabled(True)
            self.frame_number_spin_box.setReadOnly(False)

    def run(self):
        frame_number = self.frame_number_spin_box.value()
        self.visualizer.animator.go_to_frame_number(frame_number)

        if self.is_running:
            self.frame_number_spin_box.stepUp()
            QtCore.QTimer.singleShot(self.interval, self.run)
