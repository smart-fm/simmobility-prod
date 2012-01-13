#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

class Movement:
    def __init__(self, id, agent, func, args):
        self.id = id
        self.agent = agent
        self.func = func
        self.args = args

    def __call__(self):
        self.func(*self.args)

class Animator:
    def __init__(self, graphics_scene):
        self.graphics_scene = graphics_scene
        self.agents = dict()
        self.time_line = list()
        self.count = 0

    def set_pedestrian_position_at(self, agent, frame_number, position):
        id = agent.id
        if id not in self.agents:
            pedestrian = Pedestrian(id, self.graphics_scene)
            pedestrian.set_position(position)
            pedestrian.hide()
            self.agents[id] = pedestrian
        pedestrian = self.agents[id]
        self.add_movement(frame_number, id, pedestrian, pedestrian.set_position, (position,))

    def set_driver_position_and_orientation_at(self, agent, frame_number, position, angle):
        id = agent.id
        if id not in self.agents:
            driver = Driver(id, agent.length, agent.width, self.graphics_scene)
            driver.set_position_and_orientation(position, angle)
            driver.hide()
            self.agents[id] = driver
        driver = self.agents[id]
        self.add_movement(frame_number, id, driver,
                          driver.set_position_and_orientation, (position, angle))

    def add_movement(self, frame_number, id, agent, func, args):
        if frame_number == len(self.time_line):
            self.time_line.append(list())
        movements = self.time_line[frame_number]
        movements.append(Movement(id, agent, func, args))

    def go_to_frame_number(self, frame_number):
        # Any agent which did not make any movement in this frame is considered to be dead.
        # We hide all such agents.
        all_agents = self.agents.values()
        for movement in self.time_line[frame_number]:
            movement()
            all_agents.remove(movement.agent)
        for agent in all_agents:
            agent.hide()

class Driver:
    def __init__(self, id, length, width, graphics_scene):
        self.id = id
        item1 = QtGui.QGraphicsRectItem(-length / 2, -width / 2, length - 100, width)
        item2 = QtGui.QGraphicsRectItem(length / 2 - 100,  -width / 2, 100, width)
        item2.setBrush(QtCore.Qt.black)
        item = QtGui.QGraphicsItemGroup()
        item.addToGroup(item1)
        item.addToGroup(item2)
        graphics_scene.addItem(item)
        item.hide()
        self.graphics_item = item

    def set_position_and_orientation(self, position, angle):
        self.graphics_item.show()
        self.graphics_item.setPos(position.x, position.y)
        self.graphics_item.setRotation(-angle)

    def hide(self):
        self.graphics_item.hide()

class Pedestrian:
    def __init__(self, id, graphics_scene):
        self.id = id
        path = QtGui.QPainterPath(QtCore.QPointF(-40, -20))
        path.lineTo(40, 0)
        path.lineTo(-40, 20)
        path.closeSubpath()
        item = QtGui.QGraphicsPathItem(path)
        graphics_scene.addItem(item)
        item.hide()
        self.graphics_item = item

    def set_position(self, position):
        self.graphics_item.show()
        self.graphics_item.setPos(position.x, position.y)

    def hide(self):
        self.graphics_item.hide()
