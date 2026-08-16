# Konfigurations-Modul: api/-Sublib Konfiguration_API (reiner Vertrag, json-frei),
# gegen die app UND compositor linken, plus STATIC-Impl aus src/
# (KonfigurationIO.cpp: JSON <-> struct via nlohmann). nlohmann_json nur im .cpp
# -> PRIVATE_DEPENDS, damit json nicht in den Vertrag / zu Consumern leakt.
add_module(NAME Konfiguration PRIVATE_DEPENDS nlohmann_json)