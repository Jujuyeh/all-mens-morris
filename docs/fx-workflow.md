# Arduboy FX Workflow

Do not use classic Arduino upload for an Arduboy FX catalog update.
Treat Arduboy FX-C as its own target/workflow: it uses the USB-C hardware and
link-cable capable platform, and its flashcart/update path should be explicit
instead of reusing the classic FX guard by accident.

Safe flow:

1. Back up the current flashcart image.
2. Decompile the backup image.
3. Replace one explicit catalog entry with the new `.hex` and `.png`.
4. Rebuild the flashcart image.
5. Write the rebuilt image only after confirming the backup path.

This repo's `make upload` target intentionally refuses `TARGET=fx`. Use
`make fx-entry` to prepare files that can be merged into a backed-up catalog.
By default the entry is prepared in the `TableTop` category.
