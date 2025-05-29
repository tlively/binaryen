/*
 * Copyright 2025 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef wasm_ir_principal_types_h
#define wasm_ir_principal_types_h

#include <variant.h>

#include "wasm-type.h"

namespace wasm {

// The syntax of types needs to be extended with type variables for
// nullability, exactness, sharedness, heap types, and types.
using VarNullability = std::variant<Nullability, Index>;

using VarExactness = std::variant<Exactness, Index>;

enum class ExtendedShareability { Shared, Unshared, BotShared };

using VarShareability = std::variant<ExtendedShareability, Index>;

using VarHeapType = std::variant<HeapType, Index>;

struct VarRef {
  VarNullability null;
  VarExactness exact;
  VarShareability share;
  VarHeapType ht;
};

using VarType = std::variant<Type, VarRef, Index>;

struct PrincipalType {
  std::vector<VarType> rparams;
  std::vector<VarType> results;
  bool unreachable;

  bool compose(const PrincipalType& next);
  std::optional<Signature> getSignature();
};

// Lists of (variable, replacement) pairs sorted by variable.
template<typanem T> using VarAssignmentList = std::vector<std::pair<Index, T>>;

struct VarAssignments {
  VarAssignmentList<VarNullability> nulls;
  VarAssignemntList<VarExactness> exacts;
  VarAssignmentList<VarShareability> shares;
  VarAssignmentList<VarType> types;

  bool merge(const VarAssignments& other);
};

} // namespace wasm

#endif // wasm_ir_principal_types_h
