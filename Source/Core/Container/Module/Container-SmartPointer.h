#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, SmartPointer)

PARTING_IMPORT std;

PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include<memory>

#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD#pragma once

PARTING_EXPORT template<typename Type, typename Deleter = std::default_delete<Type>>
using UniquePtr = std::unique_ptr<Type, Deleter>;

PARTING_EXPORT template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
STDNODISCARD inline UniquePtr<_Ty> MakeUnique(_Types&&... _Args) { return std::make_unique<_Ty>(std::forward<_Types>(_Args)...); }

//NOTE : SharedPtr
PARTING_EXPORT template<typename Type>
using SharedPtr = std::shared_ptr<Type>;

PARTING_EXPORT template<typename Type>
using WeakPtr = std::weak_ptr<Type>;

PARTING_EXPORT template<typename Type>
using EnableSharedFromThis = std::enable_shared_from_this<Type>;


PARTING_EXPORT template<std::_Bounded_builtin_array _Ty>
STDNODISCARD inline SharedPtr<_Ty> MakeShared(void) { return std::make_shared<_Ty>(); }

PARTING_EXPORT template<std::_Bounded_builtin_array _Ty>
STDNODISCARD inline SharedPtr<_Ty> MakeShared(const std::remove_extent_t<_Ty>& _Val) { return std::make_shared<_Ty>(_Val); }

PARTING_EXPORT template<std::_Not_builtin_array _Ty, class... _Types>
STDNODISCARD inline SharedPtr<_Ty> MakeShared(_Types&&... _Args) { return std::make_shared<_Ty>(std::forward<_Types>(_Args)...); }

PARTING_EXPORT template <std::_Unbounded_builtin_array _Ty>
STDNODISCARD inline SharedPtr<_Ty> MakeShared(const size_t _Count) { return std::make_shared<_Ty>(_Count); }

PARTING_EXPORT template <std::_Unbounded_builtin_array _Ty>
STDNODISCARD inline SharedPtr<_Ty> MakeShared(const size_t _Count, const std::remove_extent_t<_Ty>& _Val) { return std::make_shared<_Ty>(_Count, _Val); }

PARTING_EXPORT template<std::_Not_builtin_array Type, typename Alloc, typename...Args>
STDNODISCARD inline SharedPtr<Type> AllocateShared(const Alloc& alloc, Args&&... args) { return std::allocate_shared<Type>(alloc, std::forward<Args>(args)...); }

PARTING_EXPORT template<std::_Bounded_builtin_array Type, typename Alloc>
STDNODISCARD inline SharedPtr<Type> AllocateShared(const Alloc& alloc) { return std::allocate_shared<Type>(alloc); }

PARTING_EXPORT template<std::_Bounded_builtin_array Type, typename Alloc>
STDNODISCARD inline SharedPtr<Type> AllocateShared(const Alloc& alloc, const std::remove_extent_t<Type>& _Val) { return std::allocate_shared<Type>(alloc, _Val); }

PARTING_EXPORT template<std::_Unbounded_builtin_array Type, typename Alloc>
STDNODISCARD inline SharedPtr<Type> AllocateShared(const Alloc& alloc, const size_t _Count) { return std::allocate_shared<Type>(alloc, _Count); }

PARTING_EXPORT template<std::_Unbounded_builtin_array Type, typename Alloc>
STDNODISCARD inline SharedPtr<Type> AllocateShared(const Alloc& alloc, const size_t _Count, const std::remove_extent_t<Type>& _Val) { return std::allocate_shared<Type>(alloc, _Count, _Val); }

//NOTE : Pointer_Cast
PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> StaticPointerCast(const SharedPtr<SrcType>& ptr) { return std::static_pointer_cast<SrcType>(ptr); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> StaticPointerCast(SharedPtr<SrcType>&& ptr) { return std::static_pointer_cast<SrcType>(std::move(ptr)); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> DynamicPointetCast(const SharedPtr<SrcType>& ptr) { return std::dynamic_pointer_cast<SrcType>(ptr); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> DynamicPointetCast(SharedPtr<SrcType>&& ptr) { return std::dynamic_pointer_cast<SrcType>(std::move(ptr)); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> ConstPointerCast(const SharedPtr<SrcType>& ptr) { return std::const_pointer_cast<SrcType>(ptr); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> ConstPointerCast(SharedPtr<SrcType>&& ptr) { return std::const_pointer_cast<SrcType>(std::move(ptr)); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> ReinterpretPointerCast(const SharedPtr<SrcType>& ptr) { return std::reinterpret_pointer_cast<SrcType>(ptr); }

PARTING_EXPORT template<typename DstType, typename SrcType>
STDNODISCARD inline SharedPtr<DstType> ReinterpretPointerCast(SharedPtr<SrcType>&& ptr) { return std::reinterpret_pointer_cast<SrcType>(std::move(ptr)); }

//NOTE: SharedPtr Must Have a delete in constructor
PARTING_EXPORT template<typename Deleter, typename Type>
STDNODISCARD inline Deleter* GetDeleter(const SharedPtr<Type>& ptr) { return std::get_deleter<Deleter>(ptr); }