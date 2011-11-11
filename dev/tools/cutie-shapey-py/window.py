#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore
import record
import selector

class GraphicsView(QtGui.QGraphicsView):
    def __init__(self, status_bar, parent=None):
        # Based on code from "Rapid GUI programming with Python and Qt" by Mark Summerfield

        super(GraphicsView, self).__init__(parent)
        self.status_bar = status_bar

        self.setDragMode(QtGui.QGraphicsView.ScrollHandDrag)
        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.setRenderHint(QtGui.QPainter.TextAntialiasing)
        self.scale(0.004, 0.004)

        # For the geospatial data, the Y-axis points upwards.  QtGui.QGraphicsView has its Y-axis
        # pointing down.  Flip the view about the X-Axis so that things look right.
        matrix = QtGui.QTransform()
        matrix.rotate(180, QtCore.Qt.XAxis)
        self.setTransform(matrix, True)

        self.setMouseTracking(True)

    def mouseMoveEvent(self, event):
        if event.buttons() == QtCore.Qt.NoButton:
            message = ""
            for item in self.items(event.x(), event.y(), 10, 10):
                message += item.info
                message += " | "
            self.status_bar.showMessage(message)
        event.ignore()
        super(GraphicsView, self).mouseMoveEvent(event)

    def wheelEvent(self, event):
        factor = 1.41 ** (-event.delta() / 240.0)
        self.scale(factor, factor)

class Main_window(QtGui.QMainWindow):
    def __init__(self, parent=None):
        super(Main_window, self).__init__(parent)
        self.setWindowTitle("Renders shape-files with PyShp and PyQt4")

        self.view = GraphicsView(self.statusBar())
        self.setCentralWidget(self.view)
        self.scene = QtGui.QGraphicsScene(self)
        self.view.setScene(self.scene)

        dock = QtGui.QDockWidget("Shape files")
        self.addDockWidget(QtCore.Qt.BottomDockWidgetArea, dock)
        self.tab_widget = QtGui.QTabWidget() 
        dock.setWidget(self.tab_widget)

        self.selector = selector.Selector(self)
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.selector)

    def add_shape_file(self, label, shape_file):
        tree = record.Record_table(shape_file)
        self.tab_widget.addTab(tree, label)
        for rec in shape_file.records():
            item = rec.graphics()
            tree.attach_item(rec, item)
            if item:
                self.scene.addItem(item)
                self.selector.add_checkable(item, rec)
        tree.enable_sorting()
        #tree.adjust_columns_width()

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
    win = Main_window()
    win.add_shape_file("Road Attributes", roads)
    win.show()

    sys.exit(app.exec_())
