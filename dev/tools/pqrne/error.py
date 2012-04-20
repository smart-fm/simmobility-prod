#!/usr/bin/env python

# Copyright (2012) Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

def show_error_message(message, parent=None):
    QtGui.QMessageBox.critical(parent, "pqrne: error message", message)

def show_info_message(message, parent=None):
    QtGui.QMessageBox.information(parent, "pqrne: informational message", message)

# vim:columns=100:smartindent:shiftwidth=4:expandtab:softtabstop=4:
