import QtQml.Models

import QGroundControl
import QGroundControl.Controls
import QGroundControl.FlyView

ToolStrip {
    id: _root

    signal displayPreFlightChecklist
    signal displayGeofenceQuickSet

    FlyViewToolStripActionList {
        id: flyViewToolStripActionList

        onDisplayPreFlightChecklist: _root.displayPreFlightChecklist()
        onDisplayGeofenceQuickSet:   _root.displayGeofenceQuickSet()
    }

    model: flyViewToolStripActionList.model
}
