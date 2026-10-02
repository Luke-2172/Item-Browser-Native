Native UI 3: corrected visible-letter vertical alignment in category tabs,
page buttons, search controls, quantity/settings buttons and footer keycaps.
Body and heading baseline offsets are calibrated from the supplied screenshot.
Alternative font replacers can override BodyBaseline (default 0.25) and
HeadingBaseline (default 0.33) under [NativeUI], range -0.5 to 0.5.
Restart after changes. Actual updated in-game appearance still needs checking.

Native UI 3 layout fixes: measured single-line text, ellipsis for long names,
measured button centering, separated text/fill depth layers and translucent fills.
Requires UIO 2.20+ for native text scaling (missing from Native UI 1 requirements).
https://www.nexusmods.com/newvegas/mods/57174

Luke's Item Browser 1.0.10-native3 - Native UI Compatibility Edition 3

This is a separate experimental rewrite using Fallout's own HUD menu XML tiles.
It is NOT the earlier EndScene/Present compatibility build. The game renders
separate text, image, border and cursor tiles. There are no Direct3D hooks and
no per-frame rasterized menu texture. Only the decorative background is a DDS.

INSTALL
Disable the existing Item Browser (standard or Compatibility 1) in MO2.
Install the FNV native3 archive as its replacement. Do not rename DLLs to load
both editions. This edition shares the DLL name, INI and MCM registration.
The archive root goes in Data and includes NVSE, MCM, menus and textures.
Install all folders. Source ZIPs are for development, not installation.
Restart the game after switching editions. Existing saves need no conversion.
To roll back, disable this edition and enable your previous browser package.

REQUIREMENTS
Fallout: New Vegas 1.4.0.525, xNVSE 6.3.11+ and JIP LN 56.95+.
MCM remains optional. Its ESP-free integration requires MCM Extender 1.63+
and that framework's dependencies. UIO 2.20+ is REQUIRED for native text scaling. Our XML
injection still uses JIP directly. No PowerShell, external font installation or UI replacer is required.

CONTROLS AND FUNCTIONS
Item: F11 or LB + D-pad Left by default.
Actor: F10 or LB + D-pad Right by default.
Existing configurable opening keys/directions, mouse/keyboard navigation,
controller prompts, typed search, categories, three columns, settings and
item/actor request validation remain. Actor values and Template toggle remain.
The browser is an unpaused native HUD panel, not a replacement for the Pip-Boy
or a new pause-menu type. It closes when a standard game menu becomes active.
Keyboard/mouse capture retains DirectInput hooks; controller blocking uses the
existing supported xNVSE/JIP commands. JIP executable code is not modified.

APPEARANCE AND SETTINGS
Uses native game font slots instead of bundled Barlow/Share Tech Mono fonts.
BodyFont=3 and HeadingFont=2 under [NativeUI] in LukesItemBrowser.ini.
Valid slots are 1-8. Font appearance depends on the game's configured fonts.
MCM's former Text resolution option now selects the native body font slot.
Restart after font changes. Display:RenderScale and Compatibility:RendererMode
from previous editions have no effect. Layout scales to the game UI viewport.
Pip-Boy colour sync, background pattern, author footer and existing sounds remain.
Text sizing/clipping requires in-game assessment with your UI/font setup.

COMPATIBILITY AND LIMITS
Each browser injects a uniquely named root under HUDMainMenu; vanilla XML files
are not overwritten. A fixed pool of 256 image/text pairs is reused. Only visible
rows are populated, and unchanged traits are not written again. XML injection
retries up to three times per load/new game and reports failure in the browser log.
The native tile adapter uses the xNVSE-documented 1.4.0.525 engine ABI and game
methods; the existing exact-runtime gate remains. It patches no rendering code.
Mods hiding the entire HUD or changing its transforms/fonts can still affect this
panel. This is not a guarantee of compatibility with every rendering/UI mod.
The shared JIP button-flag ownership limitation from standard builds remains.

VALIDATION
Native DLL builds and automated tests cover UI command capture across pages,
fixed-pool bounds, diff-only updates, text as data (not executable/XML input),
viewport validation, hide behaviour, XML/DDS structure, controller/control leases,
input-hook chaining and catalogue/actor validation. Mocks do not run the actual
Fallout XML parser or GPU. Actual in-game rendering/input remains unverified.

Please test opening, scrolling, search, item/actor actions, actor values, controller
blocking, closing, save-load and Alt-Tab. If it fails, send LukesItemBrowser.log,
nvse.log and the console printout from that launch. The log should identify
'native game UI tiles ready' once injection succeeds.
SOURCE BUILD
Run build.cmd with Visual Studio C++ x86 tools. Then run package.ps1.
NativeCheck builds the DDS assets and uses simulated tile API calls.
Legacy GDI helpers/fonts are retained for offline tests only, not game rendering.
Engine ABI references:
https://github.com/xNVSE/NVSE/blob/master/nvse/nvse/GameTiles.h
https://github.com/xNVSE/NVSE/blob/master/nvse/nvse/GameUI.cpp
https://github.com/jazzisparis/JIP-LN-NVSE/blob/main/functions_jip/jip_fn_ui.h

