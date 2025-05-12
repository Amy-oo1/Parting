#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Utility, SemanticControl)

#else 
#pragma once

//Global

#include "Core/ModuleBuild.h"

#endif // PARTING_MODULE_BUILD

//NOTE : this class used by base class,su class can not use this class
// if you device a class and use this class ,you's class will has tow base class


PARTING_EXPORT class CopyAbleOnly {
private:
	CopyAbleOnly(CopyAbleOnly&&) = delete;
	CopyAbleOnly& operator=(CopyAbleOnly&&) = delete;

protected:
	CopyAbleOnly(void) = default;

	CopyAbleOnly(const CopyAbleOnly&) = default;
	CopyAbleOnly& operator=(const CopyAbleOnly&) = default;
};

PARTING_EXPORT class MoveAbleOnly {
private:
	MoveAbleOnly(const MoveAbleOnly&) = delete;
	MoveAbleOnly& operator=(const MoveAbleOnly&) = delete;

protected:
	MoveAbleOnly(void) = default;

	MoveAbleOnly(MoveAbleOnly&&) = default;
	MoveAbleOnly& operator=(MoveAbleOnly&&) = default;

};

PARTING_EXPORT class NonCopyAndMoveAble {
private:
	NonCopyAndMoveAble(const NonCopyAndMoveAble&) = delete;
	NonCopyAndMoveAble& operator=(const NonCopyAndMoveAble&) = delete;

	NonCopyAndMoveAble(NonCopyAndMoveAble&&) = delete;
	NonCopyAndMoveAble& operator=(NonCopyAndMoveAble&&) = delete;

protected:
	NonCopyAndMoveAble(void) = default;
};