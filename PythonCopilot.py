"""Tiny CAD copilot. Build the module with -DAICADO_PYTHON=ON, then:

    import aicado
    from PythonCopilot import CADCopilot
    cad = CADCopilot(aicado.Cad())
    cad.execute({"operation": "create_box", "width": 100, "height": 50, "depth": 20})
    cad.execute({"operation": "fillet", "radius": 3})
"""


class CADCopilot:
    def __init__(self, cad):
        self.cad = cad

    def execute(self, command):
        operation = command["operation"]
        if operation == "create_box":
            ok = self.cad.create_box(command["width"], command["height"], command["depth"])
        else:
            ok = self.cad.execute(command)      # fillet, hole, extrude, ...
        if not ok:
            raise ValueError(f"{operation} failed: {self.cad.last_error()}")
        return ok
