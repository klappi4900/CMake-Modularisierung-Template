add_module(NAME Logger DEPENDS F_OPCheck_API)
# Wichtig: logger linkt NICHT gegen F_OP_Check (Implementierung),
# sondern nur gegen fileio_api (Vertrag) -> Schichtgrenze bleibt sauber.
