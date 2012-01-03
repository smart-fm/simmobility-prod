#!/usr/bin/python

# Copyright Singapore-MIT Alliance for Research and Technology

from PyQt4 import QtGui, QtCore

def create_manual():
    msg_box = QtGui.QMessageBox()
    msg_box.setWindowTitle("pqrne manual")
    msg_box.setText("""\
<h1 style="text-align: center;">pqrne</h1>
<h2>Overview</h2>
<p>pqrne is a graphical tool for editing road networks.  The lane markings and kerb lines are good candidates for the lane edges.  The editor provides facilities for the user to pick lane marking and kerb lines to be used as lane edges.</p>
<h2>Stringing lane markings to form lane edges</h2>
<p>Lane markings are not continuous along a road from one intersection to the next intersection; they change from one type to another along the way.  For example, type B changes to type C.  Whole-day bus lanes are marked by types L, A1, A2, A3, and A4.  Normal bus lanes are marked by S and S1 types.  Therefore you need to join lane markings to form a lane edge.</p>
<p>Set to <span style="color: red;">Snap to lines</span>.  Disable the <span style="color: red;">selectable</span> flags except for a minimum set of lane marking types or kerb line types.  This makes it easy for you to pick the correct lane markings.  Move the mouse over the target lane marking; the status bar will show its type (provided the type is selectable).  Right-click the lane marking to add it to the lane edge that you are trying to form.  When you have added the last section, move the mouse to an empty space and double-right-click.  A dialog box will pop up.</p>
<h2>Stringing kerb lines to form lane edges</h2>
<p>The kerb lines are usually very long and extend to the side roads.  For this reason, you shouldn't pick the entire kerb line; instead pick certain points of the kerb line (or kerb lines) to form a lane edge.  Set to <span style="color: red;">Snap to points</span> so that you only pick out points and not the lines.  When you have added the last point, move the mouse to an empty space and double-right-click.</p>
<h2>Edit the data (section-id, type, etc) of a lane edge</h2>
<p>You can change a lane edge's data at any time.  Right-click to select the edge and press <span style="color: red;">e</span>; the dialog box will appear.</p>
<h2>Delete a lane edge</h2>
<p>Right-click to select the edge and press <span style="color: red;">d</span>.</p>
<h2>Adjust the position of any point of a lane edge</h2>
<p>In many places, the lane markings do not begin at the pedestrian crossings or end at stop lines.  You can adjust any point of a lane edge.  Click and drag a point and move it to the final position.</p>
""")

    return msg_box
