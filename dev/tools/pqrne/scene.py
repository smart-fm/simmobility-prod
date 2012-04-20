#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Graphics_scene(QtGui.QGraphicsScene):
    def __init__(self, main_window):
        super(Graphics_scene, self).__init__(main_window)
        self.selector = Selector(main_window.selection_tree_widget)

    def draw_digital_map(self, digital_map):
        self.draw_roads(digital_map.roads)
        self.draw_lane_markings(digital_map.lane_markings)
        self.draw_kerb_lines(digital_map.kerb_lines)
        self.draw_arrow_markings(digital_map.arrow_markings)
        self.draw_bus_stops(digital_map.bus_stops)
        self.draw_traffic_signals(digital_map.traffic_signals)

    def draw_roads(self, roads):
        for road_name in roads:
            for road_section in roads[road_name]:
                graphics_item = self.draw_polyline(road_section)
                graphics_item.setVisible(False)
                road_section.visual_item = graphics_item
                self.selector.add_road(graphics_item)
                for point in road_section.polyline:
                    graphics_item = self.draw_point(point, road_section)
                    graphics_item.setVisible(False)
                    self.selector.add_dot(graphics_item)

    def draw_lane_markings(self, lane_markings):
        for marking_type in lane_markings:
            for lane_marking in lane_markings[marking_type]:
                self.draw_lane_marking(lane_marking)

    def draw_lane_marking(self, lane_marking):
        graphics_item = self.draw_polyline(lane_marking)
        lane_marking.visual_item = graphics_item
        self.selector.add_lane_marking(graphics_item)
        for point in lane_marking.polyline:
            graphics_item = self.draw_point(point, lane_marking)
            graphics_item.setVisible(False)
            self.selector.add_dot(graphics_item)

    def draw_kerb_lines(self, kerb_lines):
        for kerb_line in kerb_lines:
            graphics_item = self.draw_polyline(kerb_line, QtCore.Qt.green)
            kerb_line.visual_item = graphics_item
            self.selector.add_kerb_line(graphics_item)
            for point in kerb_line.polyline:
                graphics_item = self.draw_point(point, kerb_line, QtCore.Qt.green)
                graphics_item.setVisible(False)
                self.selector.add_dot(graphics_item)

    def draw_arrow_markings(self, arrow_markings):
        for arrow_marking in arrow_markings:
            if 'A' == arrow_marking.arrow_type:
                path = right_arrow_icon()
            elif 'B' == arrow_marking.arrow_type:
                path = straight_and_left_arrow_icon()
            elif 'C' == arrow_marking.arrow_type:
                path = straight_and_right_arrow_icon()
            elif 'D' == arrow_marking.arrow_type:
                path = straight_arrow_icon()
            elif 'E' == arrow_marking.arrow_type:
                path = left_arrow_icon()
            elif 'F' == arrow_marking.arrow_type:
                path = left_and_right_arrow_icon()
            elif 'G' == arrow_marking.arrow_type:
                path = conv_left_arrow_icon()
            elif 'H' == arrow_marking.arrow_type:
                path = conv_right_arrow_icon()
            elif 'I' == arrow_marking.arrow_type:
                path = straight_and_left_and_right_arrow_icon()
            else:
                path = None
            graphics_item = QtGui.QGraphicsPathItem(path)
            graphics_item.setBrush(QtCore.Qt.black)
            graphics_item.setPos(arrow_marking.position.x, arrow_marking.position.y)
            graphics_item.setRotation(-arrow_marking.bearing)
            graphics_item.setVisible(False)
            self.addItem(graphics_item)
            graphics_item.road_item = arrow_marking
            arrow_marking.visual_item = graphics_item
            graphics_item.is_selectable = True
            self.selector.add_arrow_marking(graphics_item)

    def draw_bus_stops(self, bus_stops):
        for bus_stop in bus_stops:
            path = bus_stop_icon()
            graphics_item = QtGui.QGraphicsPathItem(path)
            graphics_item.setBrush(QtCore.Qt.black)
            graphics_item.setPos(bus_stop.position.x, bus_stop.position.y)
            graphics_item.setVisible(False)
            self.addItem(graphics_item)
            graphics_item.road_item = bus_stop
            bus_stop.visual_item = graphics_item
            graphics_item.is_selectable = True
            self.selector.add_bus_stop(graphics_item)

    def draw_traffic_signals(self, traffic_signals):
        for signal in traffic_signals:
            path = traffic_signal_icon()
            graphics_item = QtGui.QGraphicsPathItem(path)
            graphics_item.setBrush(QtCore.Qt.black)
            graphics_item.setPos(signal.position.x, signal.position.y)
            graphics_item.setRotation(-signal.bearing)
            graphics_item.setVisible(False)
            self.addItem(graphics_item)
            graphics_item.road_item = signal
            signal.visual_item = graphics_item
            graphics_item.is_selectable = True
            self.selector.add_traffic_signal(graphics_item)

    def draw_road_network(self, road_network):
        self.draw_lane_edges(road_network.lane_edges)

    def draw_lane_edges(self, lane_edges):
        for marking_type in lane_edges:
            for lane_edge in lane_edges[marking_type]:
                self.draw_lane_edge(lane_edge)

    def draw_lane_edge(self, lane_edge):
        graphics_item = self.draw_polyline(lane_edge)
        graphics_item.setZValue(1)
        graphics_item.setPen(QtCore.Qt.blue)
        lane_edge.visual_item = graphics_item
        self.selector.add_lane_edge(graphics_item)

    def draw_polyline(self, road_item, pen_color=QtCore.Qt.black):
        polyline = road_item.polyline
        point = polyline[0]
        path = QtGui.QPainterPath(QtCore.QPointF(point.x, point.y))
        for point in polyline[1:]:
            path.lineTo(point.x, point.y)
        graphics_item = QtGui.QGraphicsPathItem(path)
        graphics_item.setPen(pen_color)
        self.addItem(graphics_item)
        graphics_item.road_item = road_item
        graphics_item.is_selectable = True
        return graphics_item

    def draw_point(self, point, road_item, color=QtCore.Qt.black):
        w = 0.3
        h = 0.3
        graphics_item = QtGui.QGraphicsEllipseItem(-w/2.0, -h/2.0, w, h)
        graphics_item.setPos(point.x, point.y)
        graphics_item.setPen(color)
        graphics_item.setBrush(color)
        self.addItem(graphics_item)
        graphics_item.road_item = road_item
        graphics_item.is_selectable = True
        return graphics_item

def arrow_stem_icon():
    path = QtGui.QPainterPath()
    path.addRect(-0.15, 0, 0.30, 2.40)
    return path

def arrow_head_icon():
    path = QtGui.QPainterPath(QtCore.QPointF(-0.15, 0))
    path.lineTo(0.15, 0)
    path.lineTo(0.15, 0.60)
    path.lineTo(0.45, 0.60)
    path.lineTo(0, 1.20)
    path.lineTo(-0.45, 0.60)
    path.lineTo(-0.15, 0.60)
    path.closeSubpath()
    return path

def rotated_arrow_head_icon(angle):
    matrix = QtGui.QTransform()
    matrix.rotate(angle)
    path = arrow_head_icon()
    polygon = path.toFillPolygon(matrix)
    path = QtGui.QPainterPath()
    path.addPolygon(polygon)
    return path

def right_arrow_icon():
    path1 = arrow_stem_icon()
    path2 = rotated_arrow_head_icon(-90)
    path2.translate(-0.15, 2.25)
    return path1.united(path2)

def straight_and_left_arrow_icon():
    path1 = straight_arrow_icon()
    path2 = rotated_arrow_head_icon(90)
    path2.translate(0.15, 1.50)
    return path1.united(path2)

def straight_and_right_arrow_icon():
    path1 = straight_arrow_icon()
    path2 = rotated_arrow_head_icon(-90)
    path2.translate(-0.15, 1.50)
    return path1.united(path2)

def straight_arrow_icon():
    path1 = arrow_stem_icon()
    path2 = arrow_head_icon()
    path2.translate(0, 1.80)
    return path1.united(path2)

def left_arrow_icon():
    path1 = arrow_stem_icon()
    path2 = rotated_arrow_head_icon(90)
    path2.translate(0.15, 2.25)
    return path1.united(path2)

def left_and_right_arrow_icon():
    path1 = left_arrow_icon()
    path2 = right_arrow_icon()
    return path1.united(path2)

def conv_right_arrow_icon():
    # At junction of Rochor Canal Road and Queen Street.
    path = QtGui.QPainterPath(QtCore.QPointF(0, 0))
    rect = QtCore.QRectF(-0.45, 0, 0.90, 2.10)
    path.arcTo(rect, -90, 180)
    path.lineTo(0, 2.40)
    path.lineTo(-0.60, 1.95)
    path.lineTo(0, 1.50)
    path.lineTo(0, 1.80)
    rect = QtCore.QRectF(-0.30, 0.30, 0.60, 1.50)
    path.arcTo(rect, 90, -180)
    path.closeSubpath()
    return path

def conv_left_arrow_icon():
    # At junction of Rochor Canal Road and Queen Street.
    path = QtGui.QPainterPath(QtCore.QPointF(0, 0))
    rect = QtCore.QRectF(-0.45, 0, 0.90, 2.10)
    path.arcTo(rect, -90, -180)
    path.lineTo(0, 2.40)
    path.lineTo(0.60, 1.95)
    path.lineTo(0, 1.50)
    path.lineTo(0, 1.80)
    rect = QtCore.QRectF(-0.30, 0.30, 0.60, 1.50)
    path.arcTo(rect, 90, 180)
    path.closeSubpath()
    return path

def straight_and_left_and_right_arrow_icon():
    # Found at junction of Short Street, Albert Street, and McNally Street (???).
    path1 = straight_and_left_arrow_icon()
    path2 = straight_and_right_arrow_icon()
    return path1.united(path2)

def bus_stop_icon():
    path = QtGui.QPainterPath(QtCore.QPointF(-0.15, 0))
    path.lineTo(0.15, 0)
    path.lineTo(0.15, 2.40)
    path.lineTo(1.35, 2.40)
    path.lineTo(1.35, 3.30)
    path.lineTo(-0.15, 3.30)
    path.closeSubpath()
    return path

def traffic_signal_icon():
    # The signal icon looks like an ice-cream cone, with the semi-circle representing the
    # light bulb facing the drivers or pedestrians.
    path1 = QtGui.QPainterPath()
    path1.lineTo(-0.50, 2.00)
    path1.lineTo(0.50, 2.00)
    path1.closeSubpath()
    path2 = QtGui.QPainterPath(QtCore.QPointF(-0.50, 2.00))
    path2.arcTo(-0.50, 1.50, 1.00, 1.00, 0, -180)
    path2.lineTo(0, 0)
    path2.closeSubpath()
    return path1.united(path2)

class Selector:
    def __init__(self, tree_widget):
        tree_widget.header().setResizeMode(QtGui.QHeaderView.ResizeToContents)

        self.road_group = Selector_group("Roads", tree_widget, False)
        self.lane_marking_group = Selector_group("Lane markings", tree_widget)
        self.kerb_line_group = Selector_group("Kerb lines", tree_widget)
        self.arrow_marking_group = Selector_group("Arrow markings", tree_widget, False)
        self.bus_stop_group = Selector_group("Bus stops", tree_widget, False)
        self.traffic_signal_group = Selector_group("Traffic signals", tree_widget, False)

        self.dots = list()

        self.lane_edge_group = Selector_group("Lane_edges", tree_widget)

    def add_road(self, graphics_item):
        road = graphics_item.road_item
        self.road_group.add_to_sub_group(graphics_item, road.road_name)

    def add_lane_marking(self, graphics_item):
        lane_marking = graphics_item.road_item
        self.lane_marking_group.add_to_sub_group(graphics_item, lane_marking.marking_type)

    def add_kerb_line(self, graphics_item):
        self.kerb_line_group.add(graphics_item)

    def add_arrow_marking(self, graphics_item):
        self.arrow_marking_group.add(graphics_item)

    def add_bus_stop(self, graphics_item):
        self.bus_stop_group.add(graphics_item)

    def add_traffic_signal(self, graphics_item):
        traffic_signal = graphics_item.road_item
        self.traffic_signal_group.add_to_sub_group(graphics_item, traffic_signal.signal_type)

    def add_lane_edge(self, graphics_item):
        lane_edge = graphics_item.road_item
        self.lane_edge_group.add_to_sub_group(graphics_item, lane_edge.marking_type)

    def add_dot(self, graphics_item):
        self.dots.append(graphics_item)

    def show_dots(self):
        for graphics_item in self.dots:
            if graphics_item.road_item.visual_item.isVisible():
                graphics_item.setVisible(True)

    def hide_dots(self):
        for graphics_item in self.dots:
            graphics_item.setVisible(False)

class Selector_group(QtCore.QObject):
    VISIBILITY_BUTTON_COLUMN = 1
    SELECTABILITY_BUTTON_COLUMN = 2

    def __init__(self, group_name, parent, is_initially_visible=True):
        super(Selector_group, self).__init__()

        self.tree_widget_item = QtGui.QTreeWidgetItem([group_name, "visible", "selectable"])
        if isinstance(parent, QtGui.QTreeWidget):
            parent.addTopLevelItem(self.tree_widget_item)
        else:
            parent.addChild(self.tree_widget_item)
        self.make_check_boxes(is_initially_visible)

        self.sub_groups = dict()
        self.graphics_items = list()

    def add(self, graphics_item):
        self.graphics_items.append(graphics_item)

    def add_to_sub_group(self, graphics_item, sub_group_name):
        if sub_group_name not in self.sub_groups:
            check_box = self.visibility_check_box()
            is_checked = True if check_box.checkState() == QtCore.Qt.Checked else False
            self.sub_groups[sub_group_name] = Selector_group(sub_group_name, self.tree_widget_item,
                                                             is_checked)
        sub_group = self.sub_groups[sub_group_name]
        sub_group.add(graphics_item)

    def make_check_boxes(self, is_initially_visible):
        self.make_check_box("visible", is_initially_visible,
                            Selector_group.VISIBILITY_BUTTON_COLUMN, self.toggle_visibility)
        self.make_check_box("selectable", True,
                            Selector_group.SELECTABILITY_BUTTON_COLUMN, self.toggle_selectability)

    def make_check_box(self, label, is_initially_checked, column, slot):
        check_box = QtGui.QCheckBox(label)
        check_box.setAutoFillBackground(True)
        if is_initially_checked:
            check_box.setCheckState(QtCore.Qt.Checked)
        else:
            check_box.setCheckState(QtCore.Qt.Unchecked)
        tree_widget = self.tree_widget_item.treeWidget()
        tree_widget.setItemWidget(self.tree_widget_item, column, check_box)
        check_box.group = self
        tree_widget.connect(check_box, QtCore.SIGNAL("stateChanged(int)"), slot)

    def visibility_check_box(self):
        tree_widget = self.tree_widget_item.treeWidget()
        return tree_widget.itemWidget(self.tree_widget_item,
                                      Selector_group.VISIBILITY_BUTTON_COLUMN)

    def selectability_check_box(self):
        tree_widget = self.tree_widget_item.treeWidget()
        return tree_widget.itemWidget(self.tree_widget_item,
                                      Selector_group.SELECTABILITY_BUTTON_COLUMN)

    def toggle_visibility(self, status):
        button = self.sender()
        group = button.group
        self.toggle_group_visibility(group, status)
        for sub_group in group.sub_groups.values():
            check_box = sub_group.visibility_check_box()
            check_box.setCheckState(status)

    def toggle_group_visibility(self, group, status):
        for graphics_item in group.graphics_items:
            if QtCore.Qt.Checked == status:
                graphics_item.show()
            else:
                graphics_item.hide()

    def toggle_selectability(self, status):
        button = self.sender()
        group = button.group
        self.toggle_group_selectability(group, status)
        for sub_group in group.sub_groups.values():
            check_box = sub_group.selectability_check_box()
            check_box.setCheckState(status)

    def toggle_group_selectability(self, group, status):
        for graphics_item in group.graphics_items:
            if QtCore.Qt.Checked == status:
                graphics_item.is_selectable = True
            else:
                graphics_item.is_selectable = False

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
