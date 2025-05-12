#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

#include "Core/Utility/Include/UtilityMacros.h"

PARTING_MODULE(Utility)

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Utility, SemanticControl)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Utility, Traits)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Utility, Function)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Utility, Concept)


#else 
#pragma once
#include "Core/ModuleBuild.h"

//#include "Core/Utility/Include/UtilityMacros.h" ??NOTE :if not in Module it not can in header
//Global

#include "Core/Utility/Module/Utility-SemanticControl.h"
#include "Core/Utility/Module/Utility-Traits.h"
#include "Core/Utility/Module/Utility-Function.h"
#include "Core/Utility/Module/Utility-Concept.h"

#endif // PARTING_MODULE_BUILD
