add_module(NAME Logger DEPENDS F_OPCheck_API)
# Wichtig: Logger linkt NICHT gegen F_OPCheck (Implementierung),
# sondern nur gegen F_OPCheck_API (Vertrag) -> Schichtgrenze bleibt sauber.
# Für mehr als 2 weitere Module verwende _API zum Modulnamen:
# add_module(NAME Logger DEPENDS F_OPCheck_API QT_API Explorer_API FUP_Engine_API)

