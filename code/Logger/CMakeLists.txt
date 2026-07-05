add_module(NAME Logger DEPENDS F_OPCheck_API)
# Wichtig: logger linkt NICHT gegen F_OP_Check (Implementierung),
# sondern nur gegen fileio_api (Vertrag) -> Schichtgrenze bleibt sauber.
# Für mehr als 2 weitere Module verwende _API zum Modulnamen:
# add_module(NAME Logger DEPENDS F_OPCheck_API QT_API Explorer_API FUP_Engine_API)

