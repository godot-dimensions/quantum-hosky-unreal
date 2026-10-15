#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHHandle.generated.h"

//------------------------------------------------------------------------------

/** An invalid handle; should be used when initializing handle variables. */
static const int32 QH_HANDLE_INVALID = 0;

//______________________________________________________________________________
UCLASS()
class QHGAME_API UQHHandleLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	/** Creates a new counter from which handles can be allocated. */
	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static int32 NewCounter();

	/** Allocates a handle from a counter; handles are unique per-counter. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	static int32 NewHandle(UPARAM(ref) int32& InCounter);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static bool IsValid(const int32 InHandle);
};
