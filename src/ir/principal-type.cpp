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

namespace wasm {

namespace {

bool valid(const VarRef& ref) {
  // Exactness requires a defined heap type.
  if (auto* exact = std::get_if<Exactness>(&ref.exact);
      (!exact || exact == Exact) && ref.ht.isBasic()) {
    return false;
  }
  if (auto* share = std::get_if<ExtendedShareability>(&ref.share)) {
    if (*share == BotShared) {
      // Bottom sharedness requires an abstract heap type.
      if (!ref.ht.isBasic()) {
        return false;
      }
    } else {
      // Concrete sharedness should match the type sharedness.
      if (Shareability(*share) != ref.ht.getShared()) {
        return false;
      }
    }
  } else {
    // Variable sharedness requires an abstract heap type.
    if (!ref.ht.isBasic()) {
      return false;
    }
  }
  // There should be some use of the extended syntax somewhere, otherwise this
  // could have just been a normal Type.
  bool usesNull = std::get_if<Index>(&ref.null);
  bool usesExact = std::get_if<Index>(&ref.exact);
  bool usesShare = std::get_if<Index>(&ref.share) ||
                   std::get<ExtendedShareability>(&ref.share) ==
                     ExtendedShareability::BotShared;
  if (!usesNull && !usesExact && !usesShare) {
    return false;
  }
  return true;
}

bool valid(const VarType& type) {
  if (auto* ref = std::get_if<VarRef>(&type)) {
    return valid(*ref);
  }
  if (auto* t = std::get_if<Type>(&type)) {
    return t->isSingle();
  }
  return true;
}

template<typename T>
bool mergeAssignmentList(VarAssignmentList<T>& a,
                         const VarAssignmentList<T>& b) {
  for (auto& [var, val] : b) {
    auto it =
      std::lower_bound(a.begin(), a.end(), [](const auto& x, const auto& y) {
        return x.first < y.first;
      });
    if (it != a.end() && it->first == var && it->second != val) {
      return false;
    }
    a.insert(it, {var, val});
  }
  return true;
}

// Return the variable assignments that make a match b, if such assignments
// exist.
std::optional<VarAssignments> match(const ValType& a, const ValType& b) {}

} // anonymous namespace

bool PrincipalType::compose(const PrincipalType& next) {
  // Match up the provided and required types, collecting the provided values
  // for each required variable.
  Index numProvided = results.size();
  Index numRequired = next.rparams.size();
  Index numMatched = std::min(numProvided, numRequired);
  VarAssignments assignments;
  for (Index i = 0; i < numMatched; ++i) {
    const auto& provided = results[numProvided - i - 1];
    const auto& required = next.rparams[i];
    auto newAssignments = match(provided, required);
    if (!newAssigments || !mergeAssignments(assignments, newAssignments)) {
      return false;
    }
  }

  // The matched provided and required types are consistent, so from this point
  // we know the composition will succeed and we can start mutating things. The
  // matched provided and required types annihilate each other.
  results.resize(nemProvided - numMatched);

  if (numProvided > numRequired) {

  } else if (numProvided < numRequired) {
  }
}

bool VarAssigments::merge(const VarAssignments& other) {
  if (!mergeAssignmentList(nulls, other.nulls) ||
      !mergeAssignmentList(exacts, other.exacts) ||
      !mergeAssignmentList(shares, other.shares) ||
      !mergeAssignmentList(types, other.types)) {
    return false;
  }
  return true;
}

} // namespace wasm
