#include "KimodoToolset.h"

#include "KimodoAsyncResult.h"
#include "KimodoSubsystem.h"

#include "Kismet/KismetSystemLibrary.h"
#include "ToolsetRegistry/ToolCallAsyncResultString.h"

namespace KimodoToolsetPrivate
{
	/**
	 * Fail the tool call.
	 *
	 * The registry turns a script exception raised while a tool runs into an error the agent sees.
	 * Say what went wrong and what to do instead - models recover well from a mistake and badly from
	 * not knowing they made one.
	 */
	static void Fail(const FString& Message)
	{
		UKismetSystemLibrary::RaiseScriptError(Message);
	}

	static UKimodoSubsystem* Subsystem()
	{
		UKimodoSubsystem* Kimodo = UKimodoSubsystem::Get();
		if (!Kimodo)
		{
			Fail(TEXT("MotionForge Kimodo is not available. The plugin is disabled, or this is not an "
					  "editor session."));
		}
		return Kimodo;
	}

	/**
	 * Every tool here is one call that takes seconds to minutes, so they all share this shape.
	 *
	 * Rooted because only the completion lambda refers to the result and the garbage collector
	 * cannot see that. The subsystem always invokes its callback, including on its own synchronous
	 * refusal paths, so every route through here unroots exactly once.
	 */
	static UToolCallAsyncResultString* RunStringTool(
		TFunction<void(UKimodoSubsystem&, TFunction<void(bool, const FString&)>)> Operation)
	{
		UToolCallAsyncResultString* Result = NewObject<UToolCallAsyncResultString>();
		Result->AddToRoot();

		UKimodoSubsystem* Kimodo = UKimodoSubsystem::Get();
		if (!Kimodo)
		{
			Result->SetError(TEXT("MotionForge Kimodo is not available."));
			Result->RemoveFromRoot();
			return Result;
		}

		Operation(*Kimodo, [Result](bool bSuccess, const FString& Message)
		{
			if (bSuccess)
			{
				Result->SetValue(Message);
			}
			else
			{
				Result->SetError(Message);
			}

			Result->RemoveFromRoot();
		});

		return Result;
	}
}

UToolCallAsyncResultKimodoStatus* UKimodoToolset::GetKimodoStatus()
{
	UToolCallAsyncResultKimodoStatus* Result = NewObject<UToolCallAsyncResultKimodoStatus>();
	Result->AddToRoot();

	UKimodoSubsystem* Kimodo = UKimodoSubsystem::Get();
	if (!Kimodo)
	{
		Result->SetError(TEXT("MotionForge Kimodo is not available."));
		Result->RemoveFromRoot();
		return Result;
	}

	// Never an error, even when nothing works. "Docker is not running" is the answer to the question
	// that was asked, and turning it into a tool failure would throw away the advice with it.
	Kimodo->RefreshStatus([Result](const FKimodoStatus& Status)
	{
		Result->SetValue(Status);
		Result->RemoveFromRoot();
	});

	return Result;
}

UToolCallAsyncResultString* UKimodoToolset::SetUpKimodo()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.Setup(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::StopKimodoRunner()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.StopRunner(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::RebuildKimodoRunner()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.RebuildRunner(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::CreateKimodoRetargetRig(const FString& CharacterAssetPath)
{
	return KimodoToolsetPrivate::RunStringTool(
		[CharacterAssetPath](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.CreateRetargetRig(CharacterAssetPath, MoveTemp(Done));
		});
}

FString UKimodoToolset::SetRunpodApiKey(const FString& Key)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();

	if (!Kimodo)
	{
		return TEXT("The Kimodo subsystem is not available.");
	}

	const FString Error = Kimodo->SetRunpodApiKey(Key);
	if (!Error.IsEmpty())
	{
		return Error;
	}

	return Key.TrimStartAndEnd().IsEmpty()
		? TEXT("Forgot the stored Runpod API key.")
		: TEXT("Stored. Kimodo.CloudProvision can now rent a GPU.");
}

UToolCallAsyncResultString* UKimodoToolset::CloudProvision()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.CloudProvision(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::CloudStart()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.CloudStart(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::CloudStop()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.CloudStop(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::CloudStatus()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.CloudStatus(MoveTemp(Done));
		});
}

UToolCallAsyncResultString* UKimodoToolset::CloudTerminate()
{
	return KimodoToolsetPrivate::RunStringTool(
		[](UKimodoSubsystem& Kimodo, TFunction<void(bool, const FString&)> Done)
		{
			Kimodo.CloudTerminate(MoveTemp(Done));
		});
}

FString UKimodoToolset::SetHuggingFaceToken(const FString& Token)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();

	if (!Kimodo)
	{
		return TEXT("The Kimodo subsystem is not available.");
	}

	const FString Error = Kimodo->SetHuggingFaceToken(Token);
	if (!Error.IsEmpty())
	{
		return Error;
	}

	return Token.TrimStartAndEnd().IsEmpty()
		? TEXT("Forgot the stored Hugging Face token.")
		: TEXT("Stored. Run Kimodo.Setup (or restart the runner) for it to take effect.");
}

FString UKimodoToolset::VerifyPoseConversion(const FString& MotionAssetPath, int32 Frame)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo ? Kimodo->VerifyPoseConversion(MotionAssetPath, Frame) : FString();
}

FString UKimodoToolset::CapturePoseKey(
	const FString& MotionAssetPath, int32 ClipFrame, const FString& ConstraintType)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo ? Kimodo->CapturePoseKey(MotionAssetPath, ClipFrame, ConstraintType) : FString();
}

FString UKimodoToolset::AuthorPoseConstraint(
	const FString& MotionAssetPath,
	const FString& AnimationPath,
	const FString& FrameSpec,
	const FString& ConstraintType)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo
		? Kimodo->AuthorPoseConstraint(MotionAssetPath, AnimationPath, FrameSpec, ConstraintType)
		: FString();
}

FString UKimodoToolset::ListConstraints(const FString& MotionAssetPath)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo ? Kimodo->ListConstraints(MotionAssetPath) : FString();
}

FString UKimodoToolset::ClearConstraintKeys(
	const FString& MotionAssetPath, const FString& ConstraintType, int32 ClipFrame)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo ? Kimodo->ClearConstraintKeys(MotionAssetPath, ConstraintType, ClipFrame) : FString();
}

FString UKimodoToolset::PreviewConstraintPayload(const FString& MotionAssetPath)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo ? Kimodo->PreviewConstraintPayload(MotionAssetPath) : FString();
}

FString UKimodoToolset::MeasureBones(
	const FString& AnimationPath,
	int32 Frame,
	const FString& BoneNames,
	const FString& CompareAnimationPath,
	int32 CompareFrame)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo
		? Kimodo->MeasureBones(AnimationPath, Frame, BoneNames, CompareAnimationPath, CompareFrame)
		: FString();
}

FString UKimodoToolset::DiagnoseKimodoAnimation(const FString& AnimationAssetPath)
{
	UKimodoSubsystem* Kimodo = KimodoToolsetPrivate::Subsystem();
	return Kimodo ? Kimodo->DiagnoseAnimation(AnimationAssetPath) : FString();
}

UToolCallAsyncResultString* UKimodoToolset::GetKimodoRunnerLogs(int32 Lines)
{
	UToolCallAsyncResultString* Result = NewObject<UToolCallAsyncResultString>();
	Result->AddToRoot();

	UKimodoSubsystem* Kimodo = UKimodoSubsystem::Get();
	if (!Kimodo)
	{
		Result->SetError(TEXT("MotionForge Kimodo is not available."));
		Result->RemoveFromRoot();
		return Result;
	}

	Kimodo->GetRunnerLogs(FMath::Clamp(Lines <= 0 ? 40 : Lines, 1, 2000), [Result](const FString& Logs)
	{
		Result->SetValue(Logs.IsEmpty() ? TEXT("The runner has written nothing.") : Logs);
		Result->RemoveFromRoot();
	});

	return Result;
}
