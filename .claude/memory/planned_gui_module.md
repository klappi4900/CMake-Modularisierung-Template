---
name: planned-gui-module
description: "Geplantes GUI-Modul für dieses Projekt — wird über Compositor mit eigener API eingebunden, um Austauschbarkeit der GUI-Implementierung zu ermöglichen."
metadata: 
  node_type: memory
  type: project
  originSessionId: 3632a776-1015-40ab-9ac4-da6b4e48b26f
---

Der User plant, diesem Projekt später ein GUI-Modul hinzuzufügen. `Compositor` soll die GUI über eine eigene `GUI_API` (Vertrag) einbinden, nach demselben Muster wie `IFileWriter`/`FileWriter` bzw. `ILogger`/`Logger` — d. h. GUI bekommt ein eigenes Interface, damit die konkrete GUI-Implementierung später austauschbar/ersetzbar bleibt, ohne dass `Compositor` oder `app/main.cpp` angepasst werden müssen.

**Why:** Austauschbarkeit der GUI-Implementierung ist ein explizites Ziel des Users (z. B. um später das GUI-Toolkit zu wechseln), analog zur bestehenden Vertrag/Implementierung-Trennung bei Logger und F_OPCheck.

**How to apply:** Beim Entwurf des GUI-Moduls dem etablierten Muster folgen: `code/GUI/api/` mit Interface (`IGui` o. ä.), `code/GUI/include/`+`src/` für die konkrete Implementierung, Einbindung via `add_module(NAME GUI DEPENDS ...)`. Ob GUI zusätzlich optional (`ENABLE_GUI`, per `option()` an/abschaltbar) sein soll, ist noch offen — siehe [[cmake-modularization-template]] für das generelle Optional-Modul-Muster (Guards in Compositor, Reihenfolge im Root, PUBLIC-Compile-Definition), das bei Bedarf übertragen werden kann, sobald diese Entscheidung fällt.
