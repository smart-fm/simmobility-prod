#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Graphics_view(QtGui.QGraphicsView):
    def __init__(self, parent=None):
        super(Graphics_view, self).__init__(parent)
        self.scale(0.00925, 0.00925)

        self.setDragMode(QtGui.QGraphicsView.ScrollHandDrag)
        #self.setCursor(QtGui.QCursor(QtCore.Qt.ArrowCursor))
        self.setRenderHint(QtGui.QPainter.Antialiasing)
        self.setRenderHint(QtGui.QPainter.TextAntialiasing)

        # Flip the view about the X-axis so that the view is in Cartesian co-ordinate system
        # where the Y-axis points upwards.
        matrix = QtGui.QTransform()
        matrix.rotate(180, QtCore.Qt.XAxis)
        self.setTransform(matrix, True)

        # When user presses 'C' or 'A' to rotate the view, we maintain the orientation field
        # so that we can revert to the original orientation when the user presses 'O' (letter O).
        self.orientation = 0

    def wheelEvent(self, event):
        factor = 1.41 ** (-event.delta() / 240.0)
        self.scale(factor, factor)

    def resizeEvent(self, event):
        return

        rect = self.scene().sceneRect()
        size = event.size()
        x_factor = size.width() / rect.width()
        y_factor = size.height() / rect.height()
        factor = x_factor if x_factor > y_factor else y_factor
        self.scale(factor, factor)

    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Plus:
            self.scale(1.1, 1.1)
        # too lazy to press the <Shift> to get '+'.  So let Key_Equal do the same thing.
        elif event.key() == QtCore.Qt.Key_Equal:
            self.scale(1.1, 1.1)
        elif event.key() == QtCore.Qt.Key_Minus:
            self.scale(0.91, 0.91)
        # Make it difficult to rotate the view by assigning capital 'C', 'A", and 'O' (letter O)
        elif event.key() == QtCore.Qt.Key_C and event.modifiers() == QtCore.Qt.ShiftModifier:
            # Rotate the view clockwise by 1 degree
            self.rotate(-1)
            self.orientation -= 1
        elif event.key() == QtCore.Qt.Key_A and event.modifiers() == QtCore.Qt.ShiftModifier:
            # Rotate the view anti-clockwise by 1 degree
            self.rotate(1)
            self.orientation += 1
        elif event.key() == QtCore.Qt.Key_O and event.modifiers() == QtCore.Qt.ShiftModifier:
            # Revert back to the original orientation
            self.rotate(-self.orientation)
            self.orientation = 0
