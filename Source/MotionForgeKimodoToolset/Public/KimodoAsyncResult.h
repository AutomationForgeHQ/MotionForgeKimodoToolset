// Typed promises for the setup tools, which all take a while.

#pragma once

#include "CoreMinimal.h"
#include "KimodoTypes.h"
#include "ToolsetRegistry/ToolCallAsyncResult.h"
#include "KimodoAsyncResult.generated.h"

/**
 * An async tool call that completes with Kimodo's full status.
 *
 * The registry finds the schema by reflecting over the property literally named Value, which is why
 * this exists rather than reusing the string result: an agent asking why generation is broken should
 * get the evidence as fields it can reason about, not a sentence it has to parse.
 */
UCLASS(BlueprintType)
class MOTIONFORGEKIMODOTOOLSET_API UToolCallAsyncResultKimodoStatus : public UToolCallAsyncResult
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Kimodo")
	bool SetValue(const FKimodoStatus& InValue)
	{
		return MaybeBroadcastSuccessfulCompletion(FKimodoStatus(InValue), Value);
	}

	UPROPERTY(BlueprintReadOnly, Category = "Kimodo")
	FKimodoStatus Value;
};
