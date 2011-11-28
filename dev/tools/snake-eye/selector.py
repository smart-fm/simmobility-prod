#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Row(QtGui.QTreeWidgetItem):
    def __init__(self, text, data=None):
        super(Row, self).__init__([text])
        self.children = list()
        self.data = data

    def __iadd__(self, child):
        self.addChild(child)
        self.children.append(child)
        return self

    def add(self, child):
        self.addChild(child)
        self.children.append(child)

    def hide(self):
        for child in self.children:
            if child.data:
                child.data.hide()
            child.hide()

    def show(self):
        for child in self.children:
            if child.data:
                child.data.show()
            child.show()

class Selector(QtGui.QDockWidget):
    def __init__(self, parent=None):
        super(Selector, self).__init__("Selector", parent)
        self.setFeatures(self.features() & ~QtGui.QDockWidget.DockWidgetClosable)

        widget = QtGui.QWidget()
        self.setWidget(widget)

        self.tree = QtGui.QTreeWidget()
        self.tree.setHeaderHidden(True)

        self.polylines = list()
        self.crossings = list()
        self.kerb_lines = list()
        self.stop_lines = list()

        layout = QtGui.QVBoxLayout()
        button = QtGui.QPushButton("Clear selection")
        layout.addWidget(button)
        layout.addWidget(self.tree, 1)
        layout.addLayout(self.make_checkboxes())
        widget.setLayout(layout)

        self.connect(button, QtCore.SIGNAL("clicked()"), self.clear_selection)
        self.connect(self.tree, QtCore.SIGNAL("itemSelectionChanged()"), self.selection_changed)

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(100)
        self.connect(self.timer, QtCore.SIGNAL("timeout()"), self.blink)
        self.on = True

    def make_checkboxes(self):
        layout = QtGui.QVBoxLayout()

        button = QtGui.QCheckBox("polylines")
        layout.addWidget(button)
        self.toggle_polylines_button = button
        self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_polylines)

        button = QtGui.QCheckBox("crossings")
        layout.addWidget(button)
        self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_crossings)
        self.toggle_crossings_button = button
        button.setCheckState(QtCore.Qt.Checked)

        button = QtGui.QCheckBox("kerb-lines")
        layout.addWidget(button)
        self.toggle_kerb_lines_button = button
        self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_kerb_lines)

        button = QtGui.QCheckBox("stop-lines")
        layout.addWidget(button)
        self.toggle_stop_lines_button = button
        self.connect(button, QtCore.SIGNAL("stateChanged(int)"), self.toggle_stop_lines)

        return layout

    def add_road(self, row):
        self.tree.addTopLevelItem(row)

    def clear_selection(self):
        selection_model = self.tree.selectionModel()
        selection_model.clearSelection()

    def selection_changed(self):
        items = self.tree.selectedItems()
        if len(items) == 0:
            self.stop_blinking()
        else:
            self.timer.start()

    def blink(self):
        item = self.tree.selectedItems()[0]
        if not item:
            return
        if self.on:
            self.on = False
            item.hide()
        else:
            self.on = True
            item.show()

    def stop_blinking(self):
        self.timer.stop()

    def toggle_polylines(self):
        hide = self.are_polylines_hidden()
        for item in self.polylines:
            item.hide() if hide else item.show()

    def are_polylines_hidden(self):
        return self.toggle_polylines_button.checkState() == QtCore.Qt.Unchecked

    def toggle_crossings(self):
        hide = self.are_crossings_hidden()
        for item in self.crossings:
            item.hide() if hide else item.show()

    def are_crossings_hidden(self):
        return self.toggle_crossings_button.checkState() == QtCore.Qt.Unchecked

    def toggle_kerb_lines(self):
        hide = self.are_kerb_lines_hidden()
        for item in self.kerb_lines:
            item.hide() if hide else item.show()

    def are_kerb_lines_hidden(self):
        return self.toggle_kerb_lines_button.checkState() == QtCore.Qt.Unchecked

    def toggle_stop_lines(self):
        hide = self.are_stop_lines_hidden()
        for item in self.stop_lines:
            item.hide() if hide else item.show()

    def are_stop_lines_hidden(self):
        return self.toggle_stop_lines_button.checkState() == QtCore.Qt.Unchecked
