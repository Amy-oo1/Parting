#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(Container)

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Optional)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Tuple)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(SmartPointer)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Array)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Vector)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(List)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Queue)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Variant)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Expected)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Span)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(BitSet)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Functional)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Hash)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Map)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Container/Module/Container-Optional.h"
#include "Core/Container/Module/Container-Tuple.h"
#include "Core/Container/Module/Container-SmartPointer.h"
#include "Core/Container/Module/Container-Array.h"
#include "Core/Container/Module/Container-Vector.h"
#include "Core/Container/Module/Container-Queue.h"
#include "Core/Container/Module/Container-List.h"
#include "Core/Container/Module/Container-Variant.h"
#include "Core/Container/Module/Container-Expected .h"
#include "Core/Container/Module/Container-Span.h"
#include "Core/Container/Module/Container-BitSet.h"
#include "Core/Container/Module/Container-Functional.h"
#include "Core/Container/Module/Container-Hash.h"
#include "Core/Container/Module/Container-Map.h"

#endif // PARTING_MODULE_BUILD