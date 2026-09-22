# MotionForgeKimodoToolset

Every released version of MotionForgeKimodoToolset, newest first. A release publishes **one** section of this
file — the one whose heading matches its tag — as its release notes; for an `open` plugin those
notes are posted to Discord `#releases` automatically. Write for someone who installs the plugin,
not for the commit log.

Headings are `## <x.y.z> — <date>`. Use `Added` / `Changed` / `Fixed` / `Compatibility` /
`Known issues`, only the ones that apply.

## 0.3.2 — 2026-09-22

### Changed
- The version this toolset reports to an agent is now read from the plugin's own
  descriptor rather than repeated in C++, so it can no longer answer a number the
  installed package does not carry.
- The skill catches up with the plugin: a beat is at most ten seconds and a clip sixty, Generate
  starts a stopped runner by itself, Llama 3 access is measured separately from the token, and a
  rented GPU bills while it is up.

### Compatibility
- Rebuilt against MotionForge 0.4.0 and MotionForgeKimodo 0.5.0, which it needs.

## 0.3.1 — 2026-09-08
- Packaging fix: the release now carries everything the register allows. `BuildPlugin`'s filter excludes `Config/` and every `public_extra` path, so earlier zips shipped without them.
- `GetToolsetVersion()` answers this plugin's real version; it had drifted from the descriptor.

## 0.3.0 — 2026-09-07
- Kimodo now requires a runner that speaks the contract
- Every plugin now points at kovati.dev

## 0.2.1 — 2026-08-29
- A stopped container is no longer reported as a running one

## 0.2.0 — 2026-08-29
- Apache-2.0, and a release publishes its source
- The status says whether the keys exist, and the descriptions name Drifted
- Every plugin descriptor agrees with its release tag, and says who made it

## 0.1.0 — 2026-08-28
- Free local motion generation, proven end to end; rent a GPU from the editor and make it fast
- Prove the pose converter and fix the four things that broke it
- Send authored poses and prove the payload against the binary
- Measure where bones actually ended up
- Capture a constraint key from the character you posed
- Let constraint authoring be read back and undone
- Write down what today changed, and keep beats where they belong
