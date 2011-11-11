#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
from point import Point

class Blinker:
    def __init__(self, item):
        self.count = 0
        self.item = item
        #QtCore.QTimer.singleShot(500, self, QtCore.SLOT("timeout()"))
        QtCore.QTimer.singleShot(500, self.timeout)

    def timeout(self):
        self.count += 1
        if self.item.isVisible():
            self.item.hide()
        else:
            self.item.show()
        if self.count < 10:
            #QtCore.QTimer.singleShot(500, self, QtCore.SLOT("timeout()"))
            QtCore.QTimer.singleShot(500, self.timeout)

class Record_table(QtGui.QTreeWidget):
    def __init__(self, shape_file, parent=None):
        super(Record_table, self).__init__(parent)
        self.setAlternatingRowColors(True)
        self.setIndentation(0)
        self.setHeaderLabels(shape_file.column_names())
#        self.adjust_columns_width()

        records = shape_file.records()
        for rec in records:
            self.insert_row(rec.columns(), rec.tip_columns(), rec.tips())

        self.connect(self, QtCore.SIGNAL("itemSelectionChanged()"), self.selection_changed)

    def insert_row(self, data, tip_columns, tips):
        columns = list()
        for item in data:
            if isinstance(item, Point):
                columns.append("(%d, %d)" % (item.x, item.y))
            elif isinstance(item, list):
                for thing in item:
                    if isinstance(thing, Point):
                        columns.append("(%d, %d)" % (thing.x, thing.y))
            elif not isinstance(item, str):
                columns.append(str(item))
            else:
                columns.append(item)

        if len(columns) > self.columnCount():
            self.add_columns(len(columns) - self.columnCount())

        row = QtGui.QTreeWidgetItem(columns)
        for column, tip in zip(tip_columns, tips):
            row.setToolTip(column, tip)
        self.addTopLevelItem(row)

    def adjust_columns_width(self):
        header = self.header()
        for i in range(header.count()):
            header.setResizeMode(i, QtGui.QHeaderView.ResizeToContents)

    def add_columns(self, more):
        header = self.headerItem()
        labels = list()
        for i in range(header.columnCount()):
            label = header.text(i)
            labels.append(label)
        for i in range(more):
            labels.append(label)
        self.setHeaderLabels(labels)

        #self.adjust_columns_width()

    def enable_sorting(self):
        self.setSortingEnabled(True)
        self.sortByColumn(0, QtCore.Qt.AscendingOrder)

    def attach_item(self, rec, graphics):
        row = self.topLevelItem(rec.id)
        if row:
            row.graphics = graphics
        else:
            print rec.id

    def selection_changed(self):
        items = self.selectedItems()
        if len(items):
            self.blinker = Blinker(items[0].graphics)

if "__main__" == __name__:
    def error():
        print "Usage: %s data-folder" % sys.argv[0]
        print "must specify the folder containing the road-attributes shape files"
        sys.exit(1)

    import sys
    if len(sys.argv) != 2:
        error()

    import road
    import shapefile
    try:
        roads = road.Roads("%s/RoadAttributeLine_15Sep10_104252" % sys.argv[1])
    except shapefile.ShapefileException as reason:
        print reason
        error()

    app = QtGui.QApplication(sys.argv)
    window = Record_table(roads)
    window.show()

    sys.exit(app.exec_())
