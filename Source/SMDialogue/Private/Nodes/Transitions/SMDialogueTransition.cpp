// Copyright Recursoft LLC 2019-2020. All Rights Reserved.
#include "SMDialogueTransition.h"
#include "SMDialogueChoiceNode.h"
#include "SMDialogueUtils.h"


USMDialogueTransition::USMDialogueTransition() : Super()
{
#if WITH_EDITORONLY_DATA
	bUseCustomColors = true;
	NodeColor = FLinearColor(0.3f, 0.9f, 0.55f, 1.f);
#endif
}

bool USMDialogueTransition::IsGoingToChoiceNode() const
{
	bool bSuccess = false;
	if(USMStateInstance_Base* Next = GetNextStateInstance())
	{
		bSuccess = Next->GetClass()->IsChildOf<USMDialogueChoiceNode>();

		if (!bSuccess)
		{
			TArray<USMDialogueNode*> OutNodes;
			USMDialogueUtils::GetAllConnectedDialogueNodes(Next, OutNodes, false);
			for (USMDialogueNode* OutNode : OutNodes)
			{
				// This could have regular dialogue nodes too, verify a choice is present.
				if (OutNode->GetClass()->IsChildOf<USMDialogueChoiceNode>())
				{
					bSuccess = true;
					break;
				}
			}
		}
	}

	return bSuccess;
}

bool USMDialogueTransition::IsLeavingChoiceNode() const
{
	if (USMStateInstance_Base* Previous = GetPreviousStateInstance())
	{
		return Previous->GetClass()->IsChildOf<USMDialogueChoiceNode>();
	}

	return false;
}

bool USMDialogueTransition::IsGoingToDialogueNode(bool bIncludeChoiceAsDialogue) const
{
	bool bSuccess = false;
	if (USMStateInstance_Base* Next = GetNextStateInstance())
	{
		bSuccess = bIncludeChoiceAsDialogue ? Next->GetClass()->IsChildOf<USMDialogueNode_Base>() : Next->GetClass()->IsChildOf<USMDialogueNode>();
		if (!bSuccess)
		{
			TArray<USMDialogueNode*> OutNodes;
			USMDialogueUtils::GetAllConnectedDialogueNodes(Next, OutNodes, !bIncludeChoiceAsDialogue);
			bSuccess = OutNodes.Num() > 0;
		}
	}

	return bSuccess;
}

bool USMDialogueTransition::IsLeavingDialogueNode(bool bIncludeChoiceAsDialogue) const
{
	if (USMStateInstance_Base* Previous = GetPreviousStateInstance())
	{
		return bIncludeChoiceAsDialogue ? Previous->GetClass()->IsChildOf<USMDialogueNode_Base>() : Previous->GetClass()->IsChildOf<USMDialogueNode>();
	}

	return false;
}

void USMDialogueTransition::OnTransitionInitialized_Implementation()
{
	Super::OnTransitionInitialized_Implementation();

	if(IsGoingToChoiceNode())
	{
		SetCanEvaluate(false);
	}

	bCanGoToNextDialogue = false;
}

bool USMDialogueTransition::CanEnterTransition_Implementation() const
{
	if(IsLeavingChoiceNode() || IsGoingToChoiceNode())
	{
		return true;
	}

	if(!bCanGoToNextDialogue)
	{
		return false;
	}
	
	if (USMStateInstance_Base* PreviousState = GetPreviousStateInstance())
	{
		// Don't want to exit a state machine early.
		if (PreviousState->IsStateMachine())
		{
			return PreviousState->IsInEndState();
		}
	}
	
	return true;
}

void USMDialogueTransition::OnTransitionEntered_Implementation()
{
	Super::OnTransitionEntered_Implementation();

	if (USMDialogueNode_Base* Previous = Cast<USMDialogueNode_Base>(GetPreviousStateInstance()))
	{
		if (USMDialogueNode_Base* Next = Cast<USMDialogueNode_Base>(GetNextStateInstance()))
		{
			Next->SetPreviousDialogueSpeaker(Previous->GetDialogueSpeaker());
		}
	}
}

void USMDialogueTransition::OnDialogueUpdated_Implementation()
{
	USMStateInstance_Base* PreviousInstance = GetPreviousStateInstance();
	if(PreviousInstance->IsStateMachine())
	{
		bCanGoToNextDialogue = PreviousInstance->IsInEndState();
		return;
	}

	bCanGoToNextDialogue = true;
}
