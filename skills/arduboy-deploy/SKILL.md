---
name: arduboy-deploy
description: Compile, package, and upload Arduboy projects. Use when Codex needs to build stable/debug Arduboy sketches, upload to a classic Arduboy over Arduino CLI, or prepare an Arduboy FX catalog entry without treating FX as a classic upload target.
---

# Arduboy Deploy

Use the project Makefile as the source of truth.

## Common Commands

Compile stable:

```sh
nix develop -c make compile
```

Compile debug:

```sh
nix develop -c make compile-debug
```

Prepare an Arduboy FX entry:

```sh
nix develop -c make fx-entry
```

## Safety Rules

- Never use classic Arduino upload for Arduboy FX catalog updates.
- Always back up an FX flashcart before replacing a catalog entry.
- Keep backups and generated images out of git.
- Require explicit confirmation before writing any full FX flashcart image.
