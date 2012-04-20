#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

import math
from PyQt4 import QtGui, QtCore
from collections import namedtuple

from scraping import Scraping
from edit import Edit_form, Multiple_edit_form
from error import show_error_message
from network import Lane_edge

class Graphics_view(QtGui.QGraphicsView):
    def __init__(self, graphics_scene, main_window):
        super(Graphics_view, self).__init__(graphics_scene, main_window)
        self.scale(0.925, 0.925)
        self.status_bar = main_window.statusBar()
        self.main_window = main_window

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

        self.setMouseTracking(True)
        self.selections = Selections(self)
        self.scraping = None
        self.edit_form = None
        self.snap_to_line = True

    def wheelEvent(self, event):
        if event.modifiers() == QtCore.Qt.ControlModifier:
            self.rotate(event.delta() / 240.0)
        else:
            self.zoom(1.41 ** (-event.delta() / 240.0))

    def rotate(self, delta_angle):
        self.orientation = self.orientation + delta_angle
        super(Graphics_view, self).rotate(delta_angle)

    def zoom(self, factor):
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

    def mouseMoveEvent(self, event):
        if self.selections.rubber_band.isVisible(): 
            self.selections.adjust_rubber_band(event.pos())
        elif self.scraping:
            point = self.mapToScene(event.pos())
            self.scraping.adjust(point.x(), point.y())

        if event.buttons() == QtCore.Qt.NoButton:
            point = self.mapToScene(event.pos())
            message = "(%.2f, %.2f)" % (point.x(), point.y())
            for graphics_item in self.items(event.x(), event.y(), 5, 5):
                if not graphics_item.is_selectable:
                    continue
                message = message + " | %s" % graphics_item.road_item
            self.status_bar.showMessage(message)
        event.ignore()
        super(Graphics_view, self).mouseMoveEvent(event)

    def mousePressEvent(self, event):
        if event.button() & QtCore.Qt.RightButton:
            if self.edit_form:
                show_error_message("You are currently editing one or more road-items.  "
                    "Click on the <Ok> or <Cancel> button to close the edit-form before "
                    "selecting another item.")
                super(Graphics_view, self).mousePressEvent(event)
                return

            if self.scraping:
                self.mouse_press_events_for_scraping(event)
                super(Graphics_view, self).mousePressEvent(event)
                return

            if event.modifiers() == (QtCore.Qt.ControlModifier | QtCore.Qt.AltModifier):
                self.selections.show_rubber_band(event.pos())
            elif event.modifiers() != QtCore.Qt.ControlModifier:
                self.mouse_press_events_for_scraping(event)
            else:
                if event.modifiers() != QtCore.Qt.ShiftModifier:
                    self.selections.reset_selected_items()
                for graphics_item in self.items(event.x(), event.y(), 5, 5):
                    if not graphics_item.is_selectable:
                        continue
                    self.selections.add_item(graphics_item)

                road_info = ''
                for selected_item in self.selections.selected_items:
                    road_info = road_info + selected_item.graphics_item.road_item.road_info()
                self.main_window.show_selected_road_items_info(road_info)
        super(Graphics_view, self).mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        if self.selections.rubber_band.isVisible():
            self.selections.set_rubber_band(self)
        super(Graphics_view, self).mouseReleaseEvent(event)

    def mouseDoubleClickEvent(self, event):
        if self.scraping:
            self.scraping.finish()
            self.connect(self.scraping, QtCore.SIGNAL("finish_scraping"), self.finish_scraping)
        super(Graphics_view, self).mouseDoubleClickEvent(event)

    def finish_scraping(self):
        self.scraping = None

    def mouse_press_events_for_scraping(self, event):
        point_item = None
        line_item = None    # A graphics_item with 0 z-value and is not a point.
        lane_edge_item = None # A graphics_item with non-zero z-value
        for graphics_item in self.items(event.x(), event.y(), 5, 5):
            if not graphics_item.is_selectable:
                continue
            if isinstance(graphics_item, QtGui.QGraphicsEllipseItem) and not point_item:
                point_item = graphics_item
            elif graphics_item.zValue() > 0 and not lane_edge_item:
                lane_edge_item = graphics_item
            else:
                line_item = graphics_item
            if point_item and line_item and lane_edge_item:
                break

        if not point_item and not line_item and not lane_edge_item:
            return

        self.selections.reset_selected_items()
        self.selections.reset_selection_region()

        if not self.scraping:
            self.scraping = Scraping(self.main_window)
        #elif event.modifiers() == QtCore.Qt.ControlModifier:
        #    if lane_edge_item:
        #        self.scraping.extend_to_meet(lane_edge_item)
        #    elif line_item:
        #        self.scraping.extend_to_meet(line_item)
        #    return

        #if point_item:
        #    self.scraping.add_point_item(point_item)
        #elif line_item:
        #    self.scraping.add_line_item(line_item)
        if not self.scraping.add(point_item, line_item, lane_edge_item, self.snap_to_line):
            self.scraping = None

    def edit_selected_items(self):
        if len(self.selections.selected_items) == 0:
            show_error_message("Select a road-item and try again.")
            return
        if len(self.selections.selected_items) == 1:
            road_item = self.selections.selected_items[0].graphics_item.road_item
            self.edit_form = Edit_form(road_item, self.main_window)
        else:
            #road_items = [item.graphics_item.road_item for item in self.selections.selected_items]
            #self.edit_form = Multiple_edit_form(road_items, self.main_window)
            return
        self.connect(self.edit_form, QtCore.SIGNAL("finish_editing"), self.finish_editing)
        self.edit_form.setVisible(True)

    def finish_editing(self):
        self.edit_form = None
        self.selections.reset_selected_items()

    def delete_selected_item(self):
        if len(self.selections.selected_items) != 1:
            show_error_message("You must select a lane-edge to delete.")
            return
        selected_item = self.selections.selected_items[0]
        graphics_item = selected_item.graphics_item
        if not isinstance(graphics_item.road_item, Lane_edge):
            show_error_message("You must select a lane-edge to delete.")
            return
        self.selections.reset_selected_items()
        self.scene().removeItem(graphics_item)
        lane_edge = graphics_item.road_item
        lane_edge.delete()

Selected_item = namedtuple("Selected_item", "graphics_item pen brush")

class Selections:
    def __init__(self, graphics_view):
        self.graphics_scene = graphics_view.scene()
        self.selected_items = list()
        self.selection_region = None
        self.rubber_band = QtGui.QRubberBand(QtGui.QRubberBand.Rectangle, graphics_view)

    def add_item(self, graphics_item):
        if self.selection_region:
            self.reset_selection_region()
        pen = graphics_item.pen()
        brush = graphics_item.brush()
        self.selected_items.append(Selected_item(graphics_item, pen, brush))
        if pen.style() != QtCore.Qt.NoPen:
            graphics_item.setPen(QtCore.Qt.red)
        if brush.style() != QtCore.Qt.NoBrush:
            graphics_item.setBrush(QtCore.Qt.red)

    def show_rubber_band(self, pos):
        self.reset_selected_items()
        self.reset_selection_region()
        self.rubber_band_origin = pos
        self.rubber_band.setGeometry(QtCore.QRect(self.rubber_band_origin, QtCore.QSize()))
        self.rubber_band.show()

    def reset_selected_items(self):
        for item in self.selected_items:
            item.graphics_item.setPen(item.pen)
            item.graphics_item.setBrush(item.brush)
        self.selected_items = list()

    def reset_selection_region(self):
        if self.selection_region:
            self.graphics_scene.removeItem(self.selection_region)
        self.selection_region = None

    def adjust_rubber_band(self, pos):
        self.rubber_band.setGeometry(QtCore.QRect(self.rubber_band_origin, pos).normalized())

    def set_rubber_band(self, graphics_view):
        self.rubber_band.hide()
        polygon = graphics_view.mapToScene(self.rubber_band.geometry())
        path = QtGui.QPainterPath(polygon[0])
        for point in polygon[1:]:
            path.lineTo(point)
        if not polygon.isClosed():
            path.closeSubpath()
        graphics_item = QtGui.QGraphicsPathItem(path)
        graphics_item.setPen(QtCore.Qt.red)
        graphics_item.is_selectable = False
        self.graphics_scene.addItem(graphics_item)
        self.selection_region = graphics_item

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
