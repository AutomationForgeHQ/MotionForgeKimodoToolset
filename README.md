# MotionForge Kimodo Toolset

Installing, starting and diagnosing a local motion model, as Model Context Protocol tools.

**Status: 0.3 — compiles, registers, and is in real documented use**, covering both the local
runner and the Runpod cloud-rental path (see
[REMOTE_SETUP.md](https://kovati.dev/plugins/motionforge/)).

Adapter only. It holds no logic, forwards everything to `UKimodoSubsystem`, and can be deleted
without affecting [MotionForgeKimodo](https://kovati.dev/plugins/motionforge/).

---

## What is here, and what is not

**Not here: generation.** A Motion Definition pointed at the Kimodo provider goes through
[MotionForgeToolset](https://github.com/AutomationForgeHQ/MotionForgeToolset) like any other. That is the point of the
provider abstraction — an agent that knows how to generate motion should not have to learn a second
way to do it just because the model happens to be running on this machine.

**Here: everything specific to hosting a model yourself.** All 22 tools, grouped the way their
`Category` metadata groups them in the MCP client:

**Setup — local runner lifecycle**

| Tool | |
|---|---|
| `SetUpKimodo` | check Docker, start it, build and start the runner container |
| `StartKimodoRunner` | start an already-built container. No build, no download — seconds, not minutes |
| `StopKimodoRunner` | stop the container and free the memory its models hold |
| `RebuildKimodoRunner` | rebuild the image. For a drifted image, a wire-format mismatch, or a badly built image only |
| `CreateKimodoRetargetRig` | build the SOMA rig and an IK Retargeter for a Motion Character |
| `SetHuggingFaceToken` | store the token the gated text encoder needs, in the OS credential vault |

**Cloud rental — renting and releasing a GPU on Runpod**

| Tool | |
|---|---|
| `SetRunpodApiKey` | store a write-scoped Runpod API key so a GPU can be rented without leaving the editor |
| `CloudProvision` | rent a GPU, point this project at it, and secure it — one call. Spends the user's money |
| `CloudStart` | start the rented pod; billing resumes, ready to generate in about two minutes |
| `CloudStop` | stop the rented pod; GPU billing stops, its disk keeps costing about $0.02/hr |
| `CloudStatus` | what the rented pod is doing, which GPU it is, and what it costs per hour |
| `CloudTerminate` | destroy the rented pod and its downloaded weights — the only way to pay nothing. Not reversible |

**Diagnostics — why generation isn't working, or isn't right**

| Tool | |
|---|---|
| `GetKimodoStatus` | can it generate right now, and what to do if not. Re-checks Docker and the runner |
| `DiagnoseKimodoAnimation` | measure where hands and feet land relative to the pelvis, frame by frame |
| `VerifyPoseConversion` | check a pose round-trips through Kimodo's terms unchanged — catches axis/mapping bugs other checks miss |
| `PreviewConstraintPayload` | read the constraint JSON a definition would send, without generating anything |
| `MeasureBones` | where named bones are, in centimetres, optionally against another clip |
| `GetKimodoRunnerLogs` | recent container output — where the real reason usually is |

**Pose/constraint authoring — anchoring a generation to poses the project already has**

| Tool | |
|---|---|
| `CapturePoseKey` | capture the selected character's current pose into a constraint key at a clip frame |
| `AuthorPoseConstraint` | pose a definition's constraint keys by sampling frames from an existing animation |
| `ListConstraints` | what a definition is constrained to right now, as authored, on the project's own bones |
| `ClearConstraintKeys` | remove authored constraint keys — the undo for a capture |

`CreateKimodoRetargetRig` is the one setup tool that is a judgement call rather than a repair. Kimodo
clips land on the user's skeleton by orientation matching with no setup at all; the retargeter adds
IK goals on hands and feet, which matters only when the character's proportions differ enough from
the generator's fixed body for feet to slide. The skill tells an agent to generate a clip and look
first, rather than building five assets pre-emptively.

Plus `UKimodoSkill`, which carries what the signatures cannot: when Kimodo is the right provider to
choose, that it does not animate hands, and the handful of mundane reasons local inference fails.

## Why an agent should be able to do this

Local inference breaks for a small, boring set of reasons: Docker is closed, no GPU reached the
container, the weights are still downloading, a gated model was never accepted. Every one of them is
fixable in seconds by somebody who is told which it is, and unfixable by somebody staring at *cannot
connect to the Docker daemon*.

`GetKimodoStatus` returns one sentence of advice alongside the evidence. It never fails, even when
nothing works — *Docker is not running* is the answer to the question, and turning it into a tool
error would throw away the advice with it.

## Two things the tools cannot do for the user

Both belong in whatever an agent says before calling `SetUpKimodo` the first time on a machine:

- **The first run downloads roughly 20GB** and can take a long while.
- **The text encoder is a gated Hugging Face model.** If the user has not accepted the Llama 3
  licence and logged in, the run fails partway through a download. A `401` or *gated repository* in
  the runner logs is that, and only they can fix it.

## Layout

```
MotionForgeKimodoToolset.uplugin   editor-only; ToolsetRegistry and ModelContextProtocol
Source/MotionForgeKimodoToolset/
  KimodoToolset.*                  the 22 tools, forwarding and nothing else
  KimodoSkill.h                    when to choose Kimodo, and why it fails
  KimodoAsyncResult.h              typed promise for the status tool
```

Deliberately no dependency on `ModelContextProtocol` itself — tools register with `ToolsetRegistry`
and MCP picks them up from there.
