#include "AgGraphLibrary.h"

#include "Engine/Blueprint.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Self.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_SpawnActorFromClass.h"
#include "K2Node_MacroInstance.h"
#include "Engine/InheritableComponentHandler.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#include "EdGraphUtilities.h"
#include "Editor.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "ShaderCompiler.h"
#include "Engine/SceneCaptureCube.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Components/ShapeComponent.h"
#include "Engine/TextureRenderTargetCube.h"
#include "Engine/TextureCube.h"
#include "AssetRegistryModule.h"
#include "Misc/PackageName.h"

namespace
{
	TSet<const UBlueprint*> ConstructionTargets;

	UEdGraph* EventGraph(UBlueprint* Blueprint)
	{
		return Blueprint ? FBlueprintEditorUtils::FindEventGraph(Blueprint) : nullptr;
	}

	UEdGraph* TargetGraph(UBlueprint* Blueprint)
	{
		if (Blueprint && ConstructionTargets.Contains(Blueprint))
			return FBlueprintEditorUtils::FindUserConstructionScript(Blueprint);
		return EventGraph(Blueprint);
	}

	UEdGraphNode* FindNode(UBlueprint* Blueprint, const FString& Name)
	{
		if (UEdGraph* Graph = TargetGraph(Blueprint))
		{
			for (UEdGraphNode* Node : Graph->Nodes)
			{
				if (Node && Node->GetName() == Name)
				{
					return Node;
				}
			}
		}
		UE_LOG(LogTemp, Error, TEXT("AgGraph: node '%s' not found"), *Name);
		return nullptr;
	}

	template <typename T, typename F>
	FString Place(UBlueprint* Blueprint, int32 X, int32 Y, F&& Setup)
	{
		UEdGraph* Graph = TargetGraph(Blueprint);
		if (!Graph)
		{
			UE_LOG(LogTemp, Error, TEXT("AgGraph: no selected graph"));
			return FString();
		}
		FGraphNodeCreator<T> Creator(*Graph);
		T* Node = Creator.CreateNode();
		Node->NodePosX = X;
		Node->NodePosY = Y;
		Setup(Node);
		Creator.Finalize();
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		return Node->GetName();
	}

	USCS_Node* FindScsNode(UBlueprint* Blueprint, FName Name)
	{
		return Blueprint && Blueprint->SimpleConstructionScript ? Blueprint->SimpleConstructionScript->FindSCSNode(Name) : nullptr;
	}
}

bool UAgGraphLibrary::UseConstructionScript(UBlueprint* Blueprint, bool bUseConstructionScript)
{
	if (!Blueprint || (bUseConstructionScript && !FBlueprintEditorUtils::FindUserConstructionScript(Blueprint)))
		return false;
	if (bUseConstructionScript)
		ConstructionTargets.Add(Blueprint);
	else
		ConstructionTargets.Remove(Blueprint);
	return true;
}

void UAgGraphLibrary::ClearConstructionScript(UBlueprint* Blueprint)
{
	if (UEdGraph* Graph = Blueprint ? FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) : nullptr)
	{
		TArray<UEdGraphNode*> Nodes = Graph->Nodes;
		for (UEdGraphNode* Node : Nodes)
		{
			if (Node && !Node->IsA<UK2Node_FunctionEntry>())
				FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
}

FString UAgGraphLibrary::GetConstructionScriptEntryNode(UBlueprint* Blueprint)
{
	if (UEdGraph* Graph = Blueprint ? FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) : nullptr)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (Node && Node->IsA<UK2Node_FunctionEntry>())
				return Node->GetName();
		}
	}
	return FString();
}

bool UAgGraphLibrary::ConfigureEditorGuide(UShapeComponent* Component)
{
	if (!Component)
		return false;
	Component->bDrawOnlyIfSelected = true;
	Component->SetIsVisualizationComponent(true);
	Component->SetCanEverAffectNavigation(false);
	return true;
}

void UAgGraphLibrary::ClearEventGraph(UBlueprint* Blueprint)
{
	if (UEdGraph* Graph = EventGraph(Blueprint))
	{
		TArray<UEdGraphNode*> Nodes = Graph->Nodes;
		for (UEdGraphNode* Node : Nodes)
		{
			FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
		}
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
}

FString UAgGraphLibrary::AddEventNode(UBlueprint* Blueprint, FName EventName, int32 X, int32 Y)
{
	UFunction* Function = Blueprint && Blueprint->ParentClass ? Blueprint->ParentClass->FindFunctionByName(EventName) : nullptr;
	if (!Function)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: parent event '%s' not found"), *EventName.ToString());
		return FString();
	}
	return Place<UK2Node_Event>(Blueprint, X, Y, [&](UK2Node_Event* Node)
	{
		Node->EventReference.SetExternalMember(EventName, Function->GetOwnerClass());
		Node->bOverrideFunction = true;
	});
}

FString UAgGraphLibrary::AddCallFunctionNode(UBlueprint* Blueprint, UClass* OwnerClass, FName FunctionName, int32 X, int32 Y)
{
	UFunction* Function = OwnerClass ? OwnerClass->FindFunctionByName(FunctionName) : nullptr;
	if (!Function)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: function '%s' not found"), *FunctionName.ToString());
		return FString();
	}
	return Place<UK2Node_CallFunction>(Blueprint, X, Y, [&](UK2Node_CallFunction* Node) { Node->SetFromFunction(Function); });
}

FString UAgGraphLibrary::AddSelfVariableGetNode(UBlueprint* Blueprint, FName VariableName, int32 X, int32 Y)
{
	return Place<UK2Node_VariableGet>(Blueprint, X, Y, [&](UK2Node_VariableGet* Node) { Node->VariableReference.SetSelfMember(VariableName); });
}

FString UAgGraphLibrary::AddExternalVariableGetNode(UBlueprint* Blueprint, UClass* OwnerClass, FName VariableName, int32 X, int32 Y)
{
	UClass* Owner = (!OwnerClass || OwnerClass == Blueprint->GeneratedClass) ? Blueprint->SkeletonGeneratedClass : OwnerClass;
	return Place<UK2Node_VariableGet>(Blueprint, X, Y, [&](UK2Node_VariableGet* Node) { Node->VariableReference.SetExternalMember(VariableName, Owner); });
}

bool UAgGraphLibrary::AddObjectVariable(UBlueprint* Blueprint, FName VariableName, UClass* ObjectClass)
{
	if (!Blueprint)
	{
		return false;
	}
	FEdGraphPinType Type;
	Type.PinCategory = UEdGraphSchema_K2::PC_Object;
	Type.PinSubCategoryObject = ObjectClass ? ObjectClass : Blueprint->GeneratedClass;
	if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, VariableName, Type))
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: cannot add variable '%s'"), *VariableName.ToString());
		return false;
	}
	FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, VariableName, false);
	return true;
}

namespace
{
	bool AddScalarVariable(UBlueprint* Blueprint, FName VariableName, FName Category)
	{
		if (!Blueprint)
		{
			return false;
		}
		FEdGraphPinType Type;
		Type.PinCategory = Category;
		if (!FBlueprintEditorUtils::AddMemberVariable(Blueprint, VariableName, Type))
		{
			UE_LOG(LogTemp, Error, TEXT("AgGraph: cannot add variable '%s'"), *VariableName.ToString());
			return false;
		}
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, VariableName, false);
		return true;
	}
}

bool UAgGraphLibrary::AddFloatVariable(UBlueprint* Blueprint, FName VariableName)
{
	return AddScalarVariable(Blueprint, VariableName, UEdGraphSchema_K2::PC_Float);
}

bool UAgGraphLibrary::AddIntVariable(UBlueprint* Blueprint, FName VariableName)
{
	return AddScalarVariable(Blueprint, VariableName, UEdGraphSchema_K2::PC_Int);
}

bool UAgGraphLibrary::AddBoolVariable(UBlueprint* Blueprint, FName VariableName)
{
	return AddScalarVariable(Blueprint, VariableName, UEdGraphSchema_K2::PC_Boolean);
}

FString UAgGraphLibrary::AddExternalVariableSetNode(UBlueprint* Blueprint, UClass* OwnerClass, FName VariableName, int32 X, int32 Y)
{
	return Place<UK2Node_VariableSet>(Blueprint, X, Y, [&](UK2Node_VariableSet* Node) { Node->VariableReference.SetExternalMember(VariableName, OwnerClass); });
}

FString UAgGraphLibrary::AddSelfVariableSetNode(UBlueprint* Blueprint, FName VariableName, int32 X, int32 Y)
{
	return Place<UK2Node_VariableSet>(Blueprint, X, Y, [&](UK2Node_VariableSet* Node) { Node->VariableReference.SetSelfMember(VariableName); });
}

FString UAgGraphLibrary::AddDynamicCastNode(UBlueprint* Blueprint, UClass* TargetClass, int32 X, int32 Y)
{
	return Place<UK2Node_DynamicCast>(Blueprint, X, Y, [&](UK2Node_DynamicCast* Node) { Node->TargetType = TargetClass; });
}

FString UAgGraphLibrary::AddMakeStructNode(UBlueprint* Blueprint, UScriptStruct* Struct, int32 X, int32 Y)
{
	return Place<UK2Node_MakeStruct>(Blueprint, X, Y, [&](UK2Node_MakeStruct* Node) { Node->StructType = Struct; });
}

FString UAgGraphLibrary::AddSpawnActorNode(UBlueprint* Blueprint, UClass* ActorClass, int32 X, int32 Y)
{
	UClass* Spawned = ActorClass ? ActorClass : Blueprint->GeneratedClass;
	FString Name = Place<UK2Node_SpawnActorFromClass>(Blueprint, X, Y, [](UK2Node_SpawnActorFromClass*) {});
	UK2Node_SpawnActorFromClass* Node = Cast<UK2Node_SpawnActorFromClass>(FindNode(Blueprint, Name));
	UEdGraphPin* ClassPin = Node ? Node->GetClassPin() : nullptr;
	if (!ClassPin)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: spawn node has no class pin"));
		return FString();
	}
	ClassPin->DefaultObject = Spawned;
	Node->PinDefaultValueChanged(ClassPin);  // rebuilds the exposed-on-spawn pins
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	return Name;
}

bool UAgGraphLibrary::SetExposeOnSpawn(UBlueprint* Blueprint, FName VariableName)
{
	if (!Blueprint || FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, VariableName) == INDEX_NONE)
	{
		return false;
	}
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VariableName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
	return true;
}

FString UAgGraphLibrary::AddStandardMacroNode(UBlueprint* Blueprint, FName MacroName, int32 X, int32 Y)
{
	UBlueprint* Macros = LoadObject<UBlueprint>(nullptr, TEXT("/Engine/EditorBlueprintResources/StandardMacros.StandardMacros"));
	UEdGraph* Macro = nullptr;
	for (UEdGraph* Graph : Macros ? Macros->MacroGraphs : TArray<UEdGraph*>())
	{
		if (Graph && Graph->GetFName() == MacroName)
		{
			Macro = Graph;
		}
	}
	if (!Macro)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: standard macro '%s' not found"), *MacroName.ToString());
		return FString();
	}
	return Place<UK2Node_MacroInstance>(Blueprint, X, Y, [&](UK2Node_MacroInstance* Node) { Node->SetMacroGraph(Macro); });
}

FString UAgGraphLibrary::AddBranchNode(UBlueprint* Blueprint, int32 X, int32 Y)
{
	return Place<UK2Node_IfThenElse>(Blueprint, X, Y, [](UK2Node_IfThenElse*) {});
}

FString UAgGraphLibrary::AddSelfNode(UBlueprint* Blueprint, int32 X, int32 Y)
{
	return Place<UK2Node_Self>(Blueprint, X, Y, [](UK2Node_Self*) {});
}

bool UAgGraphLibrary::ConnectPins(UBlueprint* Blueprint, const FString& FromNode, FName FromPin, const FString& ToNode, FName ToPin)
{
	UEdGraphNode* A = FindNode(Blueprint, FromNode);
	UEdGraphNode* B = FindNode(Blueprint, ToNode);
	UEdGraphPin* PinA = A ? A->FindPin(FromPin) : nullptr;
	UEdGraphPin* PinB = B ? B->FindPin(ToPin) : nullptr;
	if (!PinA || !PinB)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: pin missing %s.%s -> %s.%s"), *FromNode, *FromPin.ToString(), *ToNode, *ToPin.ToString());
		return false;
	}
	const bool bOk = GetDefault<UEdGraphSchema_K2>()->TryCreateConnection(PinA, PinB);
	if (!bOk)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: cannot connect %s.%s -> %s.%s"), *FromNode, *FromPin.ToString(), *ToNode, *ToPin.ToString());
	}
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	return bOk;
}

bool UAgGraphLibrary::SetPinDefault(UBlueprint* Blueprint, const FString& Node, FName Pin, const FString& Value)
{
	UEdGraphNode* Found = FindNode(Blueprint, Node);
	UEdGraphPin* Target = Found ? Found->FindPin(Pin) : nullptr;
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: pin %s.%s missing"), *Node, *Pin.ToString());
		return false;
	}
	GetDefault<UEdGraphSchema_K2>()->TrySetDefaultValue(*Target, Value);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	return true;
}

TArray<FString> UAgGraphLibrary::ListPins(UBlueprint* Blueprint, const FString& Node)
{
	TArray<FString> Out;
	if (UEdGraphNode* Found = FindNode(Blueprint, Node))
	{
		for (UEdGraphPin* Pin : Found->Pins)
		{
			Out.Add(FString::Printf(TEXT("%s %s %s"), Pin->Direction == EGPD_Input ? TEXT("in") : TEXT("out"),
				*Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString()));
		}
	}
	return Out;
}

bool UAgGraphLibrary::AddComponent(UBlueprint* Blueprint, UClass* ComponentClass, FName ComponentName, FName ParentName)
{
	USimpleConstructionScript* Scs = Blueprint ? Blueprint->SimpleConstructionScript : nullptr;
	if (!Scs || !ComponentClass || FindScsNode(Blueprint, ComponentName))
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: cannot add component '%s'"), *ComponentName.ToString());
		return false;
	}
	USCS_Node* Node = Scs->CreateNode(ComponentClass, ComponentName);
	// "/Root": a new root node of this Blueprint's SCS (in a child Blueprint it attaches to the inherited root).
	USCS_Node* Parent = ParentName == FName(TEXT("/Root")) ? nullptr
		: ParentName.IsNone() ? (Scs->GetRootNodes().Num() ? Scs->GetRootNodes()[0] : nullptr) : FindScsNode(Blueprint, ParentName);
	if (Parent)
	{
		Parent->AddChildNode(Node);
	}
	else
	{
		Scs->AddNode(Node);
	}
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	return true;
}

UActorComponent* UAgGraphLibrary::GetInheritedComponentTemplate(UBlueprint* Blueprint, FName ComponentName)
{
	if (!Blueprint)
	{
		return nullptr;
	}
	// Component added by a parent Blueprint's construction script: per-child override template.
	for (UBlueprintGeneratedClass* Parent = Cast<UBlueprintGeneratedClass>(Blueprint->ParentClass); Parent;
		 Parent = Cast<UBlueprintGeneratedClass>(Parent->GetSuperClass()))
	{
		if (!Parent->SimpleConstructionScript)
		{
			continue;
		}
		for (USCS_Node* Node : Parent->SimpleConstructionScript->GetAllNodes())
		{
			if (Node && Node->GetVariableName() == ComponentName)
			{
				UInheritableComponentHandler* Handler = Blueprint->GetInheritableComponentHandler(true);
				const FComponentKey Key(Node);
				UActorComponent* Template = Handler->GetOverridenComponentTemplate(Key);
				if (!Template)
				{
					Template = Handler->CreateOverridenComponentTemplate(Key);
				}
				FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
				return Template;
			}
		}
	}
	// Native (C++) default subobject: edit it on this Blueprint's class default object.
	AActor* Defaults = Blueprint->GeneratedClass ? Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject()) : nullptr;
	if (Defaults)
	{
		for (UActorComponent* Component : Defaults->GetComponents())
		{
			if (Component && Component->GetFName() == ComponentName)
			{
				FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
				return Component;
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("AgGraph: inherited component '%s' not found"), *ComponentName.ToString());
	return nullptr;
}

UActorComponent* UAgGraphLibrary::GetComponentTemplate(UBlueprint* Blueprint, FName ComponentName)
{
	USCS_Node* Node = FindScsNode(Blueprint, ComponentName);
	return Node ? Node->ComponentTemplate : nullptr;
}

bool UAgGraphLibrary::BuildReflectionCaptures()
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World || World->FeatureLevel < ERHIFeatureLevel::SM5)
	{
		UE_LOG(LogTemp, Error, TEXT("AgGraph: no SM5 editor world for reflection captures"));
		return false;
	}
	GEditor->BuildReflectionCaptures(World);
	return true;
}

UTextureCube* UAgGraphLibrary::CaptureCubemapToAsset(FVector Location, int32 Size, const FString& PackagePath)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return nullptr;
	}
	if (GShaderCompilingManager)
	{
		GShaderCompilingManager->FinishAllCompilation();
	}
	UTextureRenderTargetCube* Target = NewObject<UTextureRenderTargetCube>(GetTransientPackage());
	Target->Init(Size, PF_FloatRGBA);
	Target->UpdateResource();
	FlushRenderingCommands();
	ASceneCaptureCube* Capture = World->SpawnActor<ASceneCaptureCube>(Location, FRotator::ZeroRotator);
	USceneCaptureComponentCube* Component = Capture->GetCaptureComponentCube();
	Component->bCaptureEveryFrame = false;
	Component->bCaptureOnMovement = false;
	Component->TextureTarget = Target;
	Component->CaptureScene();
	FlushRenderingCommands();
	UPackage* Package = CreatePackage(nullptr, *PackagePath);
	const FString Name = FPackageName::GetLongPackageAssetName(PackagePath);
	UTextureCube* Texture = Target->ConstructTextureCube(Package, Name, RF_Public | RF_Standalone);
	World->DestroyActor(Capture);
	if (Texture)
	{
		FAssetRegistryModule::AssetCreated(Texture);
		Package->MarkPackageDirty();
	}
	return Texture;
}

bool UAgGraphLibrary::ConnectPixelDepthOffset(UMaterial* Material, UMaterialExpression* Expression)
{
	if (!Material || !Expression)
	{
		return false;
	}
	Material->PixelDepthOffset.Connect(0, Expression);
	Material->PostEditChange();
	return true;
}

FString UAgGraphLibrary::CompileBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return TEXT("NoBlueprint");
	}
	FCompilerResultsLog Results;
	FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &Results);
	FString Out = StaticEnum<EBlueprintStatus>()->GetNameStringByValue(Blueprint->Status);
	for (const TSharedRef<FTokenizedMessage>& Message : Results.Messages)
	{
		Out += TEXT("\n") + Message->ToText().ToString();
	}
	return Out;
}

FString UAgGraphLibrary::ExportGraphsText(UBlueprint* Blueprint)
{
	FString Out;
	if (!Blueprint)
	{
		return Out;
	}
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	for (UEdGraph* Graph : Graphs)
	{
		TSet<UObject*> Nodes;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			Nodes.Add(Node);
		}
		FString Text;
		FEdGraphUtilities::ExportNodesToText(Nodes, Text);
		Out += FString::Printf(TEXT("=== GRAPH %s (%d nodes)\n"), *Graph->GetName(), Graph->Nodes.Num()) + Text + TEXT("\n");
	}
	return Out;
}
