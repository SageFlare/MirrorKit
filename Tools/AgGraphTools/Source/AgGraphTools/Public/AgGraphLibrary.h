#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AgGraphLibrary.generated.h"

class UBlueprint;
class UActorComponent;
class UTextureCube;
class UMaterial;
class UMaterialExpression;

/**
 * Editor-only Blueprint graph authoring for editor Python (UE 4.25 exposes no node API).
 * Nodes are addressed by their object name inside the event graph, as returned by the Add* calls.
 */
UCLASS()
class AGGRAPHTOOLS_API UAgGraphLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Removes every node from the event graph, including ghost default events. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static void ClearEventGraph(UBlueprint* Blueprint);

	/** Adds an override of a parent-class event such as ReceiveTick or ReceiveBeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddEventNode(UBlueprint* Blueprint, FName EventName, int32 X, int32 Y);

	/** Adds a call to OwnerClass::FunctionName (static library or member function). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddCallFunctionNode(UBlueprint* Blueprint, UClass* OwnerClass, FName FunctionName, int32 X, int32 Y);

	/** Adds a getter for a member of the Blueprint itself (variables and SCS components). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddSelfVariableGetNode(UBlueprint* Blueprint, FName VariableName, int32 X, int32 Y);

	/** Adds a getter for OwnerClass::VariableName with a "self" target pin; OwnerClass None = this Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddExternalVariableGetNode(UBlueprint* Blueprint, UClass* OwnerClass, FName VariableName, int32 X, int32 Y);

	/** Adds an instance-editable object reference variable; ObjectClass None = this Blueprint's own class. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool AddObjectVariable(UBlueprint* Blueprint, FName VariableName, UClass* ObjectClass);

	/** Adds an instance-editable bool variable; set its default on the CDO after compiling. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool AddBoolVariable(UBlueprint* Blueprint, FName VariableName);

	/** Adds an instance-editable float variable; set its default on the CDO after compiling. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool AddFloatVariable(UBlueprint* Blueprint, FName VariableName);

	/** Adds an instance-editable int variable; set its default on the CDO after compiling. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool AddIntVariable(UBlueprint* Blueprint, FName VariableName);

	/** Adds a setter for a member variable of the Blueprint itself (no target pin). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddSelfVariableSetNode(UBlueprint* Blueprint, FName VariableName, int32 X, int32 Y);

	/** Adds a setter for OwnerClass::VariableName; its "self" pin takes the target object. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddExternalVariableSetNode(UBlueprint* Blueprint, UClass* OwnerClass, FName VariableName, int32 X, int32 Y);

	/** Adds an impure "Cast To TargetClass" node (pins: execute, Object, then, CastFailed, As<Class>). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddDynamicCastNode(UBlueprint* Blueprint, UClass* TargetClass, int32 X, int32 Y);

	/** Adds a "Make <Struct>" node; input pins are the struct's member names (e.g. Plane: X Y Z W). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddMakeStructNode(UBlueprint* Blueprint, UScriptStruct* Struct, int32 X, int32 Y);

	/** Adds a "Spawn Actor from Class" node for ActorClass (None = this Blueprint's class); exposed-on-spawn variables become input pins. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddSpawnActorNode(UBlueprint* Blueprint, UClass* ActorClass, int32 X, int32 Y);

	/** Marks a member variable Expose on Spawn (shown as a pin on Spawn Actor nodes). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool SetExposeOnSpawn(UBlueprint* Blueprint, FName VariableName);

	/** Adds an engine standard macro instance (e.g. ForEachLoop: pins Exec, Array, LoopBody, Array Element, Array Index, Completed). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddStandardMacroNode(UBlueprint* Blueprint, FName MacroName, int32 X, int32 Y);

	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddBranchNode(UBlueprint* Blueprint, int32 X, int32 Y);

	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString AddSelfNode(UBlueprint* Blueprint, int32 X, int32 Y);

	/** Connects two pins through the K2 schema, inserting conversion nodes when needed. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool ConnectPins(UBlueprint* Blueprint, const FString& FromNode, FName FromPin, const FString& ToNode, FName ToPin);

	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool SetPinDefault(UBlueprint* Blueprint, const FString& Node, FName Pin, const FString& Value);

	/** Lists "direction name category" for every pin of a node, for script debugging. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static TArray<FString> ListPins(UBlueprint* Blueprint, const FString& Node);

	/** Adds an SCS component under ParentName (None = first root node; "/Root" = new root node). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool AddComponent(UBlueprint* Blueprint, UClass* ComponentClass, FName ComponentName, FName ParentName);

	/** Returns the SCS template of a component added to this Blueprint, for setting defaults. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static UActorComponent* GetComponentTemplate(UBlueprint* Blueprint, FName ComponentName);

	/** Override template for a component inherited from a parent Blueprint's construction script
	 *  (created on first use), or the class-default component for native components. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static UActorComponent* GetInheritedComponentTemplate(UBlueprint* Blueprint, FName ComponentName);

	/** Exports every graph (event, function, macro) of a Blueprint as T3D node text, for inspection. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString ExportGraphsText(UBlueprint* Blueprint);

	/** Builds reflection captures for the editor world into its MapBuildData (needs a real RHI). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool BuildReflectionCaptures();

	/** Captures the editor world at Location into a new TextureCube asset (e.g. /Game/Mods/X/T_Env). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static UTextureCube* CaptureCubemapToAsset(FVector Location, int32 Size, const FString& PackagePath);

	/** Connects an expression to the material's Pixel Depth Offset input (not exposed to Python in 4.25). */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static bool ConnectPixelDepthOffset(UMaterial* Material, UMaterialExpression* Expression);

	/** Compiles; returns the resulting status followed by every compiler message, one per line. */
	UFUNCTION(BlueprintCallable, Category = "AgGraph")
	static FString CompileBlueprint(UBlueprint* Blueprint);
};
