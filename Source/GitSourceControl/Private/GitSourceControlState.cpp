// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlState.h"

#define LOCTEXT_NAMESPACE "GitSourceControl.State"

int32 FGitSourceControlState::GetHistorySize() const
{
	return History.Num();
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::GetHistoryItem( int32 HistoryIndex ) const
{
	check(History.IsValidIndex(HistoryIndex));
	return History[HistoryIndex];
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::FindHistoryRevision( int32 RevisionNumber ) const
{
	for(const auto& Revision : History)
	{
		if(Revision->GetRevisionNumber() == RevisionNumber)
		{
			return Revision;
		}
	}

	return NULL;
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::GetBaseRevForMerge() const
{
	// @todo get revision of the merge-base (https://www.kernel.org/pub/software/scm/git/docs/git-merge-base.html)
	return nullptr;
}

// @todo add Slate icons for git specific states (Added vs Modified, Copied vs Conflicted...)
FName FGitSourceControlState::GetIconName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Modified:
		return FName("Subversion.CheckedOut");
	case EWorkingCopyState::Added:
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
	case EWorkingCopyState::Deleted: // @todo New dedicated icon for removed/deleted (MarkedForDelete) files : Deleted files does not always show in Editor (but need to be checked-in) : only when deleted externaly !
		return FName("Subversion.OpenForAdd");
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.NotAtHeadRevision");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot");
		UE_LOG(LogSourceControl, Log, TEXT("EWorkingCopyState::Deleted"));

	case EWorkingCopyState::Missing: // @todo Missing files does not currently show in Editor (but should probably)
		UE_LOG(LogSourceControl, Log, TEXT("EWorkingCopyState::Missing"));
//	case EWorkingCopyState::Unchanged:
		// Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	}

	return NAME_None;
}

FName FGitSourceControlState::GetSmallIconName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Unchanged:
		return FName("Subversion.CheckedOut_Small");
	case EWorkingCopyState::Added:
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
	case EWorkingCopyState::Deleted: // @todo New dedicated icon for removed/deleted (MarkedForDelete) files : Deleted files does not always show in Editor (but need to be checked-in) : only when deleted externaly !
		return FName("Subversion.OpenForAdd_Small");
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.NotAtHeadRevision_Small");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot_Small");

	case EWorkingCopyState::Missing: // @todo Missing files does not currently show in Editor (but should probably)
		UE_LOG(LogSourceControl, Log, TEXT("EWorkingCopyState::Missing"));
//	case EWorkingCopyState::Unchanged:
		// Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	}

	return NAME_None;
}

FText FGitSourceControlState::GetDisplayName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown", "Unknown");
	case EWorkingCopyState::Unchanged:
		return LOCTEXT("Unchanged", "Unchanged");
	case EWorkingCopyState::Added:
		return LOCTEXT("Added", "Added");
	case EWorkingCopyState::Deleted:
		return LOCTEXT("Deleted", "Deleted");
	case EWorkingCopyState::Modified:
		return LOCTEXT("Modified", "Modified");
	case EWorkingCopyState::Renamed:
		return LOCTEXT("Renamed", "Renamed");
	case EWorkingCopyState::Copied:
		return LOCTEXT("Copied", "Copied");
	case EWorkingCopyState::Conflicted:
		return LOCTEXT("ContentsConflict", "Contents Conflict");
	case EWorkingCopyState::Ignored:
		return LOCTEXT("Ignored", "Ignored");
	case EWorkingCopyState::Merged:
		return LOCTEXT("Merged", "Merged");
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled", "Not Under Source Control");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing", "Missing");
	}

	return FText();
}

FText FGitSourceControlState::GetDisplayTooltip() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown_Tooltip", "Unknown source control state");
	case EWorkingCopyState::Unchanged:
		return LOCTEXT("Pristine_Tooltip", "There are no modifications");
	case EWorkingCopyState::Added:
		return LOCTEXT("Added_Tooltip", "Item is scheduled for addition");
	case EWorkingCopyState::Deleted:
		return LOCTEXT("Deleted_Tooltip", "Item is scheduled for deletion");
	case EWorkingCopyState::Modified:
		return LOCTEXT("Modified_Tooltip", "Item has been modified");
	case EWorkingCopyState::Conflicted:
		return LOCTEXT("ContentsConflict_Tooltip", "The contents (as opposed to the properties) of the item conflict with updates received from the repository.");
	case EWorkingCopyState::Ignored:
		return LOCTEXT("Ignored_Tooltip", "Item is being ignored.");
	case EWorkingCopyState::Merged:
		return LOCTEXT("Merged_Tooltip", "Item has been merged.");
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled_Tooltip", "Item is not under version control.");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing_Tooltip", "Item is missing (e.g., you moved or deleted it without using Git). This also indicates that a directory is incomplete (a checkout or update was interrupted).");
	}

	return FText();
}

const FString& FGitSourceControlState::GetFilename() const
{
	return LocalFilename;
}

const FDateTime& FGitSourceControlState::GetTimeStamp() const
{
	return TimeStamp;
}

// TODO : missing ?
// @todo Test: don't show deleted as they should not appear? 
//	case EWorkingCopyState::Deleted:
//	case EWorkingCopyState::Missing:
// Deleted and Missing assets cannot appear in the Content Browser
bool FGitSourceControlState::CanCheckIn() const
{
	return WorkingCopyState == EWorkingCopyState::Added
		|| WorkingCopyState == EWorkingCopyState::Deleted
		|| WorkingCopyState == EWorkingCopyState::Modified
		|| WorkingCopyState == EWorkingCopyState::Renamed;
}

bool FGitSourceControlState::CanCheckout() const
{
	return false; // With Git all tracked files in the working copy are always already checked-out (as opposed to Perforce)
}

bool FGitSourceControlState::IsCheckedOut() const
{
	return IsSourceControlled(); // With Git all tracked files in the working copy are always checked-out (as opposed to Perforce)
}

bool FGitSourceControlState::IsCheckedOutOther(FString* Who) const
{
	return false; // Git does not lock checked-out files as Perforce does
}

bool FGitSourceControlState::IsCurrent() const
{
	return true; // @todo check the state of the HEAD versus the state of tracked branch on remote
}

bool FGitSourceControlState::IsSourceControlled() const
{
	return WorkingCopyState != EWorkingCopyState::NotControlled && WorkingCopyState != EWorkingCopyState::Ignored && WorkingCopyState != EWorkingCopyState::Unknown;
}

bool FGitSourceControlState::IsAdded() const
{
	return WorkingCopyState == EWorkingCopyState::Added;
}

bool FGitSourceControlState::IsDeleted() const
{
	return WorkingCopyState == EWorkingCopyState::Deleted;
}

bool FGitSourceControlState::IsIgnored() const
{
	return WorkingCopyState == EWorkingCopyState::Ignored;
}

bool FGitSourceControlState::CanEdit() const
{
	return true; // With Git all files in the working copy are always editable (as opposed to Perforce)
}

bool FGitSourceControlState::IsUnknown() const
{
	return WorkingCopyState == EWorkingCopyState::Unknown;
}

bool FGitSourceControlState::IsModified() const
{
	// Warning: for Perforce, a checked-out file is locked for modification (whereas with Git all tracked files are checked-out),
	// so for a clean "check-in" (commit) checked-out files unmodified should be removed from the changeset (the index)
	// http://stackoverflow.com/questions/12357971/what-does-revert-unchanged-files-mean-in-perforce
	//
	// Thus, before check-in UE4 Editor call RevertUnchangedFiles() in PromptForCheckin() and CheckinFiles().
	//
	// So here we must take care to enumerate all states that need to be commited,
	// all other will be discarded :
	//  - Unknown
	//  - Unchanged
	//  - NotControlled
	//  - Ignored
	return WorkingCopyState == EWorkingCopyState::Added
		|| WorkingCopyState == EWorkingCopyState::Deleted
		|| WorkingCopyState == EWorkingCopyState::Modified
		|| WorkingCopyState == EWorkingCopyState::Renamed
		|| WorkingCopyState == EWorkingCopyState::Copied
		|| WorkingCopyState == EWorkingCopyState::Conflicted
		|| WorkingCopyState == EWorkingCopyState::Missing;
}


bool FGitSourceControlState::CanAdd() const
{
	return WorkingCopyState == EWorkingCopyState::NotControlled;
}

bool FGitSourceControlState::IsConflicted() const
{
	return WorkingCopyState == EWorkingCopyState::Conflicted;
}

#undef LOCTEXT_NAMESPACE
