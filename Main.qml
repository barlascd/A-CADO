import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import AICADO

ApplicationWindow {
    id: win
    width: 1280; height: 800; visible: true
    title: "AICADO"

    CadController { id: cad }

    // Small labelled numeric input
    component Field: RowLayout {
        id: fld
        property string label
        property real value: 0
        Label { text: fld.label; Layout.preferredWidth: 78; elide: Text.ElideRight }
        TextField {
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            Component.onCompleted: text = fld.value
            onTextEdited: fld.value = parseFloat(text.replace(",", ".")) || 0
        }
    }

    FileDialog {
        id: dlg
        property string mode: ""
        fileMode: (mode === "open" || mode === "importStep") ? FileDialog.OpenFile : FileDialog.SaveFile
        onAccepted: {
            if (mode === "save") cad.save(selectedFile)
            else if (mode === "open") cad.open(selectedFile)
            else if (mode === "importStep") cad.importStep(selectedFile)
            else if (mode === "exportStep") cad.exportStep(selectedFile)
            else if (mode === "exportStl") cad.exportStl(selectedFile)
        }
    }
    function ask(m, filter) { dlg.mode = m; dlg.nameFilters = [filter]; dlg.open() }

    Shortcut { sequences: [StandardKey.Undo]; onActivated: cad.undo() }
    Shortcut { sequences: [StandardKey.Redo]; onActivated: cad.redo() }
    Shortcut { sequences: [StandardKey.Save]; onActivated: win.ask("save", "AICADO project (*.aicado)") }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            MenuItem { text: qsTr("New"); onTriggered: cad.newProject() }
            MenuItem { text: qsTr("Open..."); onTriggered: win.ask("open", "AICADO project (*.aicado)") }
            MenuItem { text: qsTr("Save..."); onTriggered: win.ask("save", "AICADO project (*.aicado)") }
            MenuItem { text: qsTr("Recover autosave"); onTriggered: cad.openAutosave() }
            MenuSeparator {}
            MenuItem { text: qsTr("Import STEP..."); onTriggered: win.ask("importStep", "STEP (*.step *.stp)") }
            MenuItem { text: qsTr("Export STEP..."); onTriggered: win.ask("exportStep", "STEP (*.step *.stp)") }
            MenuItem { text: qsTr("Export STL..."); onTriggered: win.ask("exportStl", "STL (*.stl)") }
            MenuSeparator {}
            MenuItem { text: qsTr("Quit"); onTriggered: Qt.quit() }
        }
        Menu {
            title: qsTr("&Edit")
            MenuItem { text: qsTr("Undo"); enabled: cad.canUndo; onTriggered: cad.undo() }
            MenuItem { text: qsTr("Redo"); enabled: cad.canRedo; onTriggered: cad.redo() }
            MenuItem { text: qsTr("Delete last feature"); onTriggered: cad.deleteLast() }
            MenuItem { text: qsTr("Clear face selection"); onTriggered: cad.clearSelection() }
        }
        Menu {
            title: qsTr("&View")
            MenuItem { text: qsTr("Fit view"); onTriggered: view.fitView() }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ScrollView {
            Layout.preferredWidth: 300; Layout.fillHeight: true
            contentWidth: availableWidth
            ColumnLayout {
                width: parent.width - 12; x: 6
                spacing: 8

                GroupBox {
                    title: qsTr("Box"); Layout.fillWidth: true
                    ColumnLayout { anchors.fill: parent
                        Field { id: bw; label: "Width"; value: 100; Layout.fillWidth: true }
                        Field { id: bh; label: "Height"; value: 50; Layout.fillWidth: true }
                        Field { id: bd; label: "Depth"; value: 20; Layout.fillWidth: true }
                        Button { text: qsTr("Add box"); Layout.fillWidth: true; onClicked: cad.createBox(bw.value, bh.value, bd.value) }
                    }
                }
                GroupBox {
                    title: qsTr("Cylinder"); Layout.fillWidth: true
                    ColumnLayout { anchors.fill: parent
                        Field { id: cr; label: "Radius"; value: 20; Layout.fillWidth: true }
                        Field { id: ch; label: "Height"; value: 40; Layout.fillWidth: true }
                        Button { text: qsTr("Add cylinder"); Layout.fillWidth: true; onClicked: cad.createCylinder(cr.value, ch.value) }
                    }
                }
                GroupBox {
                    title: qsTr("Extrude / revolve"); Layout.fillWidth: true
                    ColumnLayout { anchors.fill: parent
                        Field { id: ew; label: "Width"; value: 40; Layout.fillWidth: true }
                        Field { id: eh; label: "Height"; value: 30; Layout.fillWidth: true }
                        Field { id: ed; label: "Distance"; value: 20; Layout.fillWidth: true }
                        Field { id: ez; label: "Z offset"; value: 0; Layout.fillWidth: true }
                        CheckBox { id: ecut; text: qsTr("Cut from body") }
                        Button { text: qsTr("Extrude rectangle"); Layout.fillWidth: true
                            onClicked: cad.addFeature("extrude", {kind: 0, width: ew.value, height: eh.value, distance: ed.value, z: ez.value, cut: ecut.checked ? 1 : 0}) }
                        Button { text: qsTr("Revolve ring (r=ext. width/2)"); Layout.fillWidth: true
                            onClicked: cad.addFeature("revolve", {inner: 0, outer: ew.value / 2, height: eh.value, cut: ecut.checked ? 1 : 0}) }
                        Button { text: qsTr("Loft square -> small square"); Layout.fillWidth: true
                            onClicked: cad.addFeature("loft", {w1: ew.value, h1: ew.value, w2: ew.value / 3, h2: ew.value / 3, height: ed.value * 2}) }
                    }
                }
                GroupBox {
                    title: qsTr("Fillet / chamfer / hole"); Layout.fillWidth: true
                    ColumnLayout { anchors.fill: parent
                        Label { text: cad.selectedFace > 0 ? qsTr("Selected face: %1").arg(cad.selectedFace) : qsTr("Click a face (none = all edges)"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Field { id: fr; label: "Size"; value: 3; Layout.fillWidth: true }
                        RowLayout {
                            Button { text: qsTr("Fillet"); Layout.fillWidth: true; onClicked: cad.addFeature("fillet", {radius: fr.value}) }
                            Button { text: qsTr("Chamfer"); Layout.fillWidth: true; onClicked: cad.addFeature("chamfer", {distance: fr.value}) }
                        }
                        Field { id: hx; label: "Hole X"; value: 50; Layout.fillWidth: true }
                        Field { id: hy; label: "Hole Y"; value: 25; Layout.fillWidth: true }
                        Field { id: hd; label: "Diameter"; value: 10; Layout.fillWidth: true }
                        Field { id: hdp; label: "Depth"; value: 10; Layout.fillWidth: true }
                        CheckBox { id: hthru; text: qsTr("Through all") }
                        Button { text: qsTr("Drill hole (from top)"); Layout.fillWidth: true
                            onClicked: cad.addFeature("hole", {x: hx.value, y: hy.value, diameter: hd.value, depth: hdp.value, throughAll: hthru.checked ? 1 : 0}) }
                    }
                }
                GroupBox {
                    title: qsTr("Mirror / pattern / transform"); Layout.fillWidth: true
                    ColumnLayout { anchors.fill: parent
                        ComboBox { id: mp; model: ["YZ plane (X)", "XZ plane (Y)", "XY plane (Z)"]; Layout.fillWidth: true }
                        Field { id: mo; label: "Offset"; value: 0; Layout.fillWidth: true }
                        Button { text: qsTr("Mirror + merge"); Layout.fillWidth: true; onClicked: cad.addFeature("mirror", {plane: mp.currentIndex, offset: mo.value}) }
                        Field { id: pc; label: "Count"; value: 3; Layout.fillWidth: true }
                        Field { id: ps; label: "Spacing"; value: 120; Layout.fillWidth: true }
                        Button { text: qsTr("Linear pattern (X)"); Layout.fillWidth: true; onClicked: cad.addFeature("pattern", {count: pc.value, spacing: ps.value, axis: 0}) }
                        Field { id: mx; label: "Move X"; value: 0; Layout.fillWidth: true }
                        Field { id: my; label: "Move Y"; value: 0; Layout.fillWidth: true }
                        Field { id: mz; label: "Move Z"; value: 0; Layout.fillWidth: true }
                        Button { text: qsTr("Move body"); Layout.fillWidth: true; onClicked: cad.addFeature("move", {x: mx.value, y: my.value, z: mz.value}) }
                        Button { text: qsTr("Rotate 90° about Z"); Layout.fillWidth: true; onClicked: cad.addFeature("rotate", {angle: 90}) }
                    }
                }
                GroupBox {
                    title: qsTr("AI / JSON command"); Layout.fillWidth: true
                    ColumnLayout { anchors.fill: parent
                        TextField { id: cmd; Layout.fillWidth: true
                            placeholderText: '{"operation":"create_box","width":50,"height":50,"depth":50}'
                            onAccepted: cad.runCommand(text) }
                        Button { text: qsTr("Run"); Layout.fillWidth: true; onClicked: cad.runCommand(cmd.text) }
                    }
                }
            }
        }

        ViewportItem {
            id: view
            controller: cad
            Layout.fillWidth: true; Layout.fillHeight: true
        }

        ColumnLayout {
            Layout.preferredWidth: 240; Layout.fillHeight: true
            Label { text: qsTr("Feature tree"); font.bold: true; Layout.margins: 6 }
            ListView {
                Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                model: cad.features
                delegate: ItemDelegate { width: ListView.view.width; text: modelData; font.pixelSize: 12 }
                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8
            Label { text: cad.status; Layout.fillWidth: true; elide: Text.ElideRight }
            Label { text: cad.info; opacity: 0.8 }
        }
    }
}
