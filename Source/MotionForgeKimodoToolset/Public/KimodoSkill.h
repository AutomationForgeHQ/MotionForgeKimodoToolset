// What an agent needs to know about running a motion model locally.

#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/AgentSkill.h"
#include "KimodoSkill.generated.h"

/**
 * When to reach for Kimodo instead of a paid provider, and how to get it working.
 *
 * Deliberately short and free of tool signatures. The tools describe themselves; what belongs here
 * is the choice between providers and the shape of a local install, neither of which a signature can
 * express and neither of which a model can guess.
 */
UCLASS()
class MOTIONFORGEKIMODOTOOLSET_API UKimodoSkill : public UAgentSkill
{
	GENERATED_BODY()

public:

	UKimodoSkill()
	{
		Description = TEXT(
			"Generate character animation locally and for free with NVIDIA Kimodo through "
			"MotionForge: choosing it over a paid provider, setting up its Docker runner, and "
			"diagnosing it when it will not start.");

		Instructions = TEXT(
			"Kimodo is a motion generator that runs on the user's own machine in a Docker "
			"container. It is one MotionForge provider among several, so you generate with it "
			"through MotionForge's normal tools - set the provider to 'Kimodo' on a motion "
			"definition and everything else is unchanged. The tools in this toolset are only about "
			"getting and keeping it running.\n"
			"\n"
			"WHEN TO CHOOSE IT\n"
			"\n"
			"Kimodo costs nothing, generates in seconds, and honours a seed. Prefer it for anything "
			"exploratory: blocking out a library, iterating on wording, trying ten readings of the "
			"same action. Ask for many variants freely - there is no bill and no quota, so the "
			"careful economy a paid provider demands is wasted effort here.\n"
			"\n"
			"Because it seeds, a definition is a complete recipe. The clip can be deleted and "
			"recreated exactly, so treat generated files as a cache rather than as originals - the "
			"opposite of a provider without seeds, where a discarded take is gone forever.\n"
			"\n"
			"Two limits decide when to use something else. It generates at most ten seconds. And it "
			"does not animate hands at all: the model predicts a thirty joint skeleton with no "
			"fingers, and the hands hold one relaxed pose for the whole clip. For anything where the "
			"hands are the point - operating a control, gripping a tool, a detailed gesture - say "
			"so plainly and suggest a provider that generates on the project's own rig, or hand "
			"posing layered on afterwards.\n"
			"\n"
			"Its strengths are locomotion, posture, gross body action, and emotional or physical "
			"style. Prompts work best starting 'A person...', describing one or two behaviours, at "
			"medium detail. It has no prompt rewriter, so the wording you send is the wording it "
			"uses.\n"
			"\n"
			"BEATS, AND THE ONE RULE THAT COSTS A CLIP\n"
			"\n"
			"Kimodo divides a prompt at every full stop and generates each piece with its own "
			"duration. Two consequences, both of which bite silently.\n"
			"\n"
			"A full stop is a beat break and nothing else - not a sentence end. A decimal point "
			"divides a prompt, so 'hold for 2.5 seconds' is two beats; '!' and '?' do not divide "
			"one. Write numbers in words.\n"
			"\n"
			"And a beat inherits the body from the beat before it, but not the words. Each beat's "
			"text is evaluated without knowing what the previous beat said, so every beat must name "
			"the pose it acts on in full. 'The person holds the arm perfectly still' fails, because "
			"standing with the arms down is perfectly still and this beat cannot see the sentence "
			"that raised the arm. Name it - 'holds the right arm motionless in the previous pose, "
			"raised out in front at shoulder height' - and it holds. A beat that goes wrong also "
			"robs the beat after it, so read a bad clip beat by beat rather than watching it.\n"
			"\n"
			"Words hold a pose approximately. For exactly, author a constraint: Author Pose "
			"Constraint takes clipFrame=animFrame pairs, so '179=45' means 'at clip frame 179, be "
			"in the pose this clip was in at frame 45' - which is how you stop a limb drifting, "
			"using a pose the clip already produced. Preview the payload before spending a "
			"generation on it, and judge the result by measuring bones, never by whether the call "
			"succeeded: a constraint is a hint to a diffusion model, so the answer is a distance.\n"
			"\n"
			"HOW CLIPS REACH THE USER'S SKELETON\n"
			"\n"
			"Kimodo generates on its own fixed thirty joint rig, and there are two ways off it. By "
			"default the clip is built straight onto the user's skeleton by matching orientations. "
			"That needs no setup, no assets and no configuration, and it is what happens unless "
			"somebody chooses otherwise.\n"
			"\n"
			"What it does not do is IK. If the character's proportions differ much from the "
			"generator's fixed body, feet will slide. The rig creation tool fixes that by generating "
			"a real skeleton, two IK rigs and an IK Retargeter with goals on the hands and feet, and "
			"pointing the Motion Character at them.\n"
			"\n"
			"Do not run it pre-emptively. Suggest generating one clip, looking at the feet, and only "
			"building the rig if they slide - the two rigs may be close enough that it buys nothing "
			"and costs five assets in the project. It is reversible either way: clearing Provider "
			"Mesh on the character goes back to the direct path.\n"
			"\n"
			"WHEN IT WILL NOT WORK\n"
			"\n"
			"Check the status tool before generating with Kimodo the first time in a session, and "
			"again first thing whenever a Kimodo generation fails. It gives you one sentence of "
			"advice; act on that rather than reasoning from the other fields.\n"
			"\n"
			"Nearly every failure is one of four mundane things: Docker is not installed, Docker is "
			"installed but not running, the container is not up, or the weights are still "
			"downloading. The setup tool fixes the middle two by itself and starts the download for "
			"the last. Only the first needs the user.\n"
			"\n"
			"Warn the user before the first setup on a machine. It downloads roughly 20GB and can "
			"take a long time. It also needs them to have accepted the Llama 3 licence on Hugging "
			"Face and logged in on the host, because the text encoder is a gated model - if the "
			"runner logs mention a gated repository or a 401, that is what happened, and only they "
			"can fix it.\n"
			"\n"
			"If the status comes back working but on CPU, say so unprompted. It means no GPU "
			"reached the container, generation will take minutes rather than seconds, and on "
			"Windows it usually means WSL2 or the NVIDIA Container Toolkit is missing. It is worth "
			"fixing before generating a library, not after.\n"
			"\n"
			"When something fails and the status looks fine, read the runner logs. Out-of-memory "
			"kills, gated model downloads and missing GPU passthrough appear there and nowhere "
			"else.\n"
			"\n"
			"Do not rebuild the container as a general remedy. It is the fix for exactly three "
			"things: an Image State of Drifted on the status, meaning the built image came from "
			"different runner files than the plugin ships; a clip that fails to read with a version "
			"or joint-count mismatch; and a runner that answers while reporting Kimodo as not "
			"importable. Drifted is the only one of the three with hard evidence behind it, so read "
			"it first - a drifted container starts and passes every health check while behaving like "
			"an older plugin, which is why nothing else points at the image.\n"
			"\n"
			"Finally, the runner may not be on this machine at all. When the status says the runner "
			"is remote, none of the setup tools apply - there is nothing here to install, and "
			"whoever owns that machine has to start it.");
	}
};
