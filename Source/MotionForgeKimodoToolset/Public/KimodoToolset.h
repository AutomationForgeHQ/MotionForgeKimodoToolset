// Running a local motion model, as tools an agent can call.

#pragma once

#include "CoreMinimal.h"
#include "KimodoTypes.h"
#include "ToolsetRegistry/ToolsetDefinition.h"
#include "KimodoToolset.generated.h"

class UToolCallAsyncResultKimodoStatus;
class UToolCallAsyncResultString;

/**
 * Installing, starting and diagnosing the Kimodo runner, as Model Context Protocol tools.
 *
 * Generation is not here. A motion definition pointed at the Kimodo provider goes through
 * MotionForge's own tools like any other, which is the point of the provider abstraction - an agent
 * that knows how to generate motion does not need to learn a second way to do it just because the
 * model happens to be running on this machine.
 *
 * What is here is everything specific to hosting a model yourself: the install, the container, the
 * device it chose, and the reason it is not working. That last one matters most. Local inference
 * fails for a handful of mundane reasons - Docker closed, no GPU passthrough, weights still
 * downloading - and every one of them is fixable in seconds by somebody who is told which it is.
 *
 * Every function forwards to UKimodoSubsystem and adds nothing.
 */
UCLASS(BlueprintType)
class MOTIONFORGEKIMODOTOOLSET_API UKimodoToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:

	virtual FString GetToolsetVersion() const override { return TEXT("0.1.1"); }

	/**
	 * Report whether Kimodo can generate right now, and what to do if it cannot.
	 *
	 * **Call this before generating anything with the Kimodo provider**, and call it again first
	 * whenever a Kimodo generation fails. It re-checks Docker and the runner rather than reporting
	 * a cached answer, so it takes a second or two.
	 *
	 * Read Advice and act on it; the other fields are the evidence behind it. Device Mode is worth
	 * repeating to the user when it comes back as CPU only - that means no GPU reached the
	 * container, generation will take minutes instead of seconds, and it is usually fixable.
	 *
	 * Three more fields answer questions that would otherwise be guessed at:
	 *
	 * - **Image State** says whether the image on this machine was built from the runner this plugin
	 *   ships. `Drifted` is the one to act on - it is the deterministic signal for a container that
	 *   starts, answers, and then behaves like an older plugin. Rebuild Kimodo Runner is the fix, and
	 *   `Drifted` is the only evidence that justifies it.
	 * - **Has Hugging Face Token** false means the runner will build, start, report a device mode and
	 *   then fail every generation. Only the user can fix it, and telling them early is the whole
	 *   point of asking before generating.
	 * - **Has Runpod Api Key** false means a GPU cannot be rented. Irrelevant when running locally.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Diagnostics")
	static UToolCallAsyncResultKimodoStatus* GetKimodoStatus();

	/**
	 * Get Kimodo working: check Docker, start it if it is installed but closed, then build and
	 * start the runner container.
	 *
	 * Costs nothing and generates nothing. Safe to call repeatedly - every step checks before
	 * acting, so running it against a working install just confirms it.
	 *
	 * **Warn the user before calling this the first time on a machine.** It downloads roughly 20GB
	 * and can take a long while: a CUDA base image, PyTorch, Kimodo and an 8B text encoder. It will
	 * also fail at the download stage rather than at startup if they have not accepted the Llama 3
	 * licence on Hugging Face and logged in - if the runner logs mention a gated repository or a
	 * 401, that is what happened, and only the user can fix it.
	 *
	 * Returns what the runner reports about itself once it is up, or the reason it is not.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Setup")
	static UToolCallAsyncResultString* SetUpKimodo();

	/**
	 * Stop the local runner container.
	 *
	 * Worth suggesting when the user is done generating for a while - the container holds an 8B
	 * model resident, which is memory their machine could be using for the editor.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Setup")
	static UToolCallAsyncResultString* StopKimodoRunner();

	/**
	 * Rebuild the runner image and restart it.
	 *
	 * Not a general remedy. There are exactly three things this fixes:
	 *
	 * - **Get Kimodo Status reports Image State as `Drifted`.** The image was built from different
	 *   runner files than this plugin ships. That is the one case with hard evidence behind it, and
	 *   it is worth checking first, because the drifted container starts and answers health checks
	 *   perfectly while behaving like an older plugin.
	 * - A clip fails to read with a version or joint-count mismatch, meaning the plugin and the
	 *   container disagree about the wire format.
	 * - The runner answers but reports Kimodo as not importable, meaning the image built badly.
	 *
	 * Anything else - a slow generation, a bad-looking clip, a runner that is simply stopped - is not
	 * this. Stopping and starting the container does not rebuild it, and should be tried first.
	 *
	 * Downloaded weights live outside the image and survive, so this costs minutes rather than
	 * repeating the first run.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Setup")
	static UToolCallAsyncResultString* RebuildKimodoRunner();

	/**
	 * Build a SOMA skeleton and an IK Retargeter, and set them on a Motion Character so its clips
	 * retarget with IK rather than being mapped straight onto the skeleton.
	 *
	 * **Optional, and it is a real choice - explain it before doing it.** Without this, Kimodo
	 * clips are built directly onto the game's skeleton by matching orientations. That needs no
	 * setup, produces correct rotations, and is the default. What it lacks is IK: a character whose
	 * proportions differ from the generator's fixed body will slide its feet.
	 *
	 * With it, clips are built on Kimodo's own rig and moved across by Unreal's retargeter, which
	 * adds IK goals on the hands and feet, per-chain settings, and an editable retarget pose. The
	 * cost is five generated assets in the project and a rig the user has to understand.
	 *
	 * The honest advice is to generate a clip first, look at the feet, and only do this if they
	 * slide. Suggest that rather than running it pre-emptively.
	 *
	 * Costs nothing but takes about a minute: it generates a short throwaway clip so the rig can be
	 * recovered exactly instead of guessed. Safe to run again - assets are rebuilt in place.
	 *
	 * The character needs a Preview Mesh set. Tell the user to review the generated retargeter
	 * before they build a library on it.
	 *
	 * @param CharacterAssetPath Content path of the Motion Character asset.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Setup")
	static UToolCallAsyncResultString* CreateKimodoRetargetRig(const FString& CharacterAssetPath);

	/**
	 * Store a Hugging Face access token so Kimodo's text encoder can be downloaded.
	 *
	 * **Ask the user for the token; never invent one and never read one out of a file.** They create
	 * it at huggingface.co/settings/tokens, and read scope is enough.
	 *
	 * This exists so nobody has to install a command-line tool to use the plugin. The encoder is
	 * `Meta-Llama-3-8B-Instruct`, which is gated: without authentication the container builds,
	 * starts, reports a device mode, answers health checks, and then fails every single generation -
	 * so a missing token looks like a broken install rather than an unaccepted licence.
	 *
	 * The token goes to the OS credential vault, not to a config file and not into the project.
	 * Restart the runner afterwards for it to take effect.
	 *
	 * @param Token The access token, or empty to forget the stored one.
	 * @return Empty on success, or why it could not be stored.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Setup")
	static FString SetHuggingFaceToken(const FString& Token);

	/**
	 * Store a Runpod API key, so a GPU can be rented without leaving the editor.
	 *
	 * **Ask the user for the key.** They create it at runpod.io under Settings > API Keys, and it
	 * needs **write** access - a read-only key lists hardware happily and then fails to create
	 * anything with a 403 that reads like a billing problem.
	 *
	 * @param Key The API key, or empty to forget the stored one.
	 * @return Empty on success, or why it could not be stored.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Cloud")
	static FString SetRunpodApiKey(const FString& Key);

	/**
	 * Rent a GPU, point this project at it, and secure it - the whole hosted setup in one call.
	 *
	 * **This spends the user's money and you must have their agreement on it first.** It picks a
	 * GPU with real stock, rents it at that card's hourly rate, and bills from that moment until
	 * Cloud Stop. Say the expected rate before calling, and say Cloud Stop exists.
	 *
	 * Worth suggesting when generation is slow because the machine has no suitable GPU: the local
	 * runner takes five to seven minutes per new prompt on a CPU without bf16, against roughly
	 * twenty seconds on a rented card. It is not worth suggesting to anyone whose own GPU already
	 * has about 20GB free - they already have the fast path.
	 *
	 * Needs a Runpod API key and a Hugging Face token stored first; it refuses with an explanation
	 * otherwise. Calling it twice reuses the pod this project already has rather than renting a
	 * second one.
	 *
	 * The pod still has to install the runner and load about 17GB of weights afterwards, so poll
	 * Kimodo Status rather than assuming it is usable the moment this returns.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Cloud")
	static UToolCallAsyncResultString* CloudProvision();

	/**
	 * Start the rented pod. Billing resumes.
	 *
	 * About two minutes until it can generate - thirty seconds for the pod, ninety for the encoder
	 * to load from the volume. Nothing is re-downloaded.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Cloud")
	static UToolCallAsyncResultString* CloudStart();

	/**
	 * Stop the rented pod. The GPU stops billing; its disks keep costing about $0.02/hr.
	 *
	 * **Suggest this whenever hosted generation is finished for the session.** A running pod bills
	 * while idle and the failure mode is silent - roughly $317 a month for a card nobody is using.
	 *
	 * Be accurate about what it saves: this takes the bill from ~$0.44/hr to ~$0.02/hr, not to zero.
	 * The remaining charge holds the volume and its ~17GB of weights, which is what makes starting
	 * it again a two-minute job. Only Cloud Terminate reaches zero.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Cloud")
	static UToolCallAsyncResultString* CloudStop();

	/** What the rented pod is doing, which GPU it is, and what it costs per hour. */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Cloud")
	static UToolCallAsyncResultString* CloudStatus();

	/**
	 * Destroy the rented pod and everything on it, including about 17GB of downloaded weights.
	 *
	 * **Not the same as stopping, and not reversible. Confirm with the user before calling.**
	 *
	 * This is the only way to stop paying entirely - a stopped pod still costs about $0.02/hr, some
	 * $14 a month, for the disks it is holding. Worth it when the next generation session is weeks
	 * out; not worth it overnight, because the next provision rebuilds everything and re-downloads
	 * ~17GB, which is ten to fifteen minutes rather than two.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Cloud")
	static UToolCallAsyncResultString* CloudTerminate();

	/**
	 * Measure what an imported clip actually does - where each hand and foot ends up relative to
	 * the pelvis, and at which frame.
	 *
	 * **Use this instead of judging a generated animation by description.** "The arm looks wrong"
	 * narrows nothing down. "The right hand peaks 12cm above the pelvis when the generator put it
	 * at 104cm" says how large the error is and which way it points, which is the difference
	 * between fixing a retarget and guessing at it.
	 *
	 * Costs nothing and changes nothing. Worth running on the first clip for any new character or
	 * bone map, and any time a result looks off.
	 *
	 * @param AnimationAssetPath Content path of the animation, e.g. the Imported Sequence Path from
	 *        Get Motion Status.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Diagnostics")
	static FString DiagnoseKimodoAnimation(const FString& AnimationAssetPath);

	/**
	 * Check that a pose on the project's skeleton converts into Kimodo's terms correctly.
	 *
	 * Free, instant, and needs no GPU. A retargeted clip and the take it was generated from are the
	 * same motion on two skeletons, so converting the first back must reproduce the second - any
	 * disagreement is the pose converter's fault.
	 *
	 * **Run this before trusting authored constraints**, and again after any change to the bone map,
	 * the rig, or the axis conversions. Two coordinate bugs have shipped in this pipeline that every
	 * other measurement passed; this is the check that would have caught a third.
	 *
	 * Under about two degrees mean is working. Tens of degrees, or one joint far worse than the
	 * others, is a mapping or an axis problem rather than retarget noise.
	 *
	 * @param MotionAssetPath Motion Definition with a selected take and an imported animation. It
	 *                        names its own character, so there is nothing else to pass and no way to
	 *                        accidentally measure against a rest pose the clip was not built on.
	 * @param Frame           Frame to compare, or -1 for several across the clip. Naming one frame
	 *                        also lists every joint separately, which is what turns "18 degrees out"
	 *                        into a diagnosis.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Diagnostics")
	static FString VerifyPoseConversion(const FString& MotionAssetPath, int32 Frame);

	/**
	 * Pose a definition's constraint keys from an existing animation.
	 *
	 * Kimodo's real advantage over a text-only generator: say *where the body is* at particular
	 * frames and let the model invent everything between them. The anchors are usually poses the
	 * project already has - the idle a clip should start from, the idle it should settle back into -
	 * so they can be sampled rather than authored.
	 *
	 * Sparse beats dense. Under twenty keys per constraint; past that the model spends its capacity
	 * satisfying constraints instead of producing motion. Constraints must not contradict the prompt
	 * or each other, which produces artifacts rather than errors.
	 *
	 * Writes to the definition and sends nothing. Read the result with Preview Constraint Payload.
	 *
	 * @param MotionAssetPath Definition to write the keys into.
	 * @param AnimationPath   Animation to sample. Must be on the character's target skeleton.
	 * @param FrameSpec       `clipFrame=animFrame` pairs, comma separated. "0=0,119=0" pins a clip to
	 *                        start and end on the animation's first frame.
	 * @param ConstraintType  fullbody, left-hand, right-hand, left-foot, right-foot, root2d.
	 */
	/**
	 * Capture the selected character's pose, as it looks right now, into a constraint key.
	 *
	 * The visual half of authoring, and the one that covers the shot-specific beat - the hand on the
	 * elevator button - which no existing animation contains. Pose the character however suits:
	 * Control Rig, a Sequencer scrub, an animation preview. This reads the evaluated skeleton off
	 * the component, so it does not care which.
	 *
	 * Repeatable. Capture at frame 0, then 75, then 119 and you have one constraint with three keys;
	 * capturing again at a frame that already has one replaces it rather than pinning the clip to
	 * two poses at the same instant.
	 *
	 * @param MotionAssetPath Definition to write the key into.
	 * @param ClipFrame       Which frame of the generated clip this pins. 30fps.
	 * @param ConstraintType  fullbody, left-hand, right-hand, left-foot, right-foot, root2d.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Authoring")
	static FString CapturePoseKey(
		const FString& MotionAssetPath,
		int32 ClipFrame,
		const FString& ConstraintType);

	UFUNCTION(meta = (AICallable), Category = "Kimodo|Authoring")
	static FString AuthorPoseConstraint(
		const FString& MotionAssetPath,
		const FString& AnimationPath,
		const FString& FrameSpec,
		const FString& ConstraintType);

	/**
	 * What a definition is constrained to right now, as authored.
	 *
	 * Read this after any capture. Authoring writes into the asset and says one line about it; this is
	 * how you confirm the key landed on the frame you meant, off the character you meant, with a pose
	 * in it. Every key lists where its pose came from, which is what tells a mis-capture - the right
	 * frame off the wrong actor - from a mis-typed frame.
	 *
	 * Shows authored numbers on the project's own bones. `PreviewConstraintPayload` shows the same
	 * keys converted into the runner's terms; if this reads correctly and that does not, the fault is
	 * the conversion rather than the authoring.
	 *
	 * @param MotionAssetPath Definition to read.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Authoring")
	static FString ListConstraints(const FString& MotionAssetPath);

	/**
	 * Remove authored constraint keys.
	 *
	 * The undo for a capture, and the way out of a definition that has picked up a key at the wrong
	 * frame or off the wrong pose - which is the normal first attempt, not an unusual accident.
	 *
	 * Clearing every key of a type removes the constraint itself, so a cleared definition reads as
	 * having no control settings rather than as having an empty constraint that pins nothing.
	 *
	 * @param MotionAssetPath Definition to clear keys from.
	 * @param ConstraintType  fullbody, left-hand, right-hand, left-foot, right-foot, root2d, or empty
	 *                        for every type.
	 * @param ClipFrame       A single clip frame to drop, or -1 for every key of that type.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Authoring")
	static FString ClearConstraintKeys(
		const FString& MotionAssetPath,
		const FString& ConstraintType,
		int32 ClipFrame);

	/**
	 * Read the constraint JSON a definition would send, without generating anything.
	 *
	 * Free and instant. `VerifyPoseConversion` proves the maths; this proves the wiring - that the
	 * converted numbers reach the request under the names the runner reads. Those fail separately,
	 * and the wiring has been wrong before while the maths was right.
	 *
	 * Worth reading before spending a generation on a constraint: hip height around 0.9 standing,
	 * ground positions in metres starting at zero, thirty axis-angle triples per pose.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Diagnostics")
	static FString PreviewConstraintPayload(const FString& MotionAssetPath);

	/**
	 * Where named bones are, in centimetres, optionally measured against another clip.
	 *
	 * How to tell whether a constraint worked. Kimodo's constraints are a hint to a diffusion model
	 * rather than a solver, so the answer is a distance, not a yes - and a distance only means
	 * something next to the same measurement from an unconstrained take of the same prompt.
	 *
	 * @param AnimationPath        The clip to measure.
	 * @param Frame                Which frame of it.
	 * @param BoneNames            Comma separated, e.g. "hand_r,hand_l,head".
	 * @param CompareAnimationPath Optional second clip; empty just reports positions.
	 * @param CompareFrame         Which frame of that one.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Diagnostics")
	static FString MeasureBones(
		const FString& AnimationPath,
		int32 Frame,
		const FString& BoneNames,
		const FString& CompareAnimationPath,
		int32 CompareFrame);

	/**
	 * Read recent output from the runner container.
	 *
	 * Where the real reason lives when a generation fails and the status looks fine. Out-of-memory
	 * kills, gated Hugging Face repositories and missing GPU passthrough all show up here and
	 * nowhere else.
	 *
	 * @param Lines How many lines from the end. Forty is usually enough; ask for more when chasing
	 *        something that happened during startup.
	 */
	UFUNCTION(meta = (AICallable), Category = "Kimodo|Diagnostics")
	static UToolCallAsyncResultString* GetKimodoRunnerLogs(int32 Lines);
};
