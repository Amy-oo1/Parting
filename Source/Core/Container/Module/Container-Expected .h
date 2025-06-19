#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Container, Expected)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<variant>
#include<type_traits>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

#include <string>
#include <stdexcept>
#include <type_traits>

// My std::expected
template <typename ValueType, typename ErrorType>
class Expected {
public:
	template <typename OtherType = ValueType, std::enable_if_t<!std::is_same_v<std::decay_t<OtherType>, Expected>>* = nullptr>
	Expected(OtherType&& value) : m_Data{ std::forward<OtherType>(value) } {}

	template <typename G = ErrorType, std::enable_if_t<!std::is_same_v<std::decay_t<G>, Expected>>* = nullptr>
	Expected(const ErrorType& error) : m_Data{ error } {}

	Expected(ErrorType&& error) : m_Data(std::move(error)) {}

	STDNODISCARD bool HasValue(void) const noexcept { return std::holds_alternative<ValueType>(this->m_Data); }

	ValueType& Value(void) {
		ASSERT(this->HasValue());
		return std::get<ValueType>(this->m_Data);
	}

	const ValueType& Value(void) const {
		ASSERT(this->HasValue());
		return std::get<ValueType>(this->m_Data);
	}

	ErrorType& Error(void) {
		ASSERT(!this->HasValue());
		return std::get<ErrorType>(this->m_Data);
	}

	const ErrorType& Error(void) const {
		ASSERT(!this->HasValue());
		return std::get<ErrorType>(this->m_Data);
	}

private:
	std::variant<ValueType, ErrorType> m_Data;
};
