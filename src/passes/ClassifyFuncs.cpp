/*
 * Copyright 2024 WebAssembly Community Group participants
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

#include "pass.h"
#include "wasm-traversal.h"
#include "wasm.h"

namespace wasm {

struct ClassifyFuncs : Pass {

  std::optional<std::string> getModule(std::string file) {
    auto jsIndex = file.find(".js");
    if (jsIndex == std::string::npos) {
      return std::nullopt;
    }
    std::stringstream escaped;
    for (auto c : file.substr(0, jsIndex)) {
      if (c == '/') {
        escaped << '-';
      } else {
        escaped << c;
      }
    }
    return escaped.str();
  }

  std::optional<Function::DebugLocation> getFirstDebugLoc(Function& func) {
    if (!func.body) {
      return {};
    }
    if (func.prologLocation.size()) {
      return *func.prologLocation.begin();
    }
    struct LocFinder
      : PostWalker<LocFinder, UnifiedExpressionVisitor<LocFinder>> {
      Function& func;
      std::optional<Function::DebugLocation> loc;
      LocFinder(Function& func) : func(func) {}
      void visitExpression(Expression* curr) {
        if (loc) {
          return;
        }
        if (auto it = func.debugLocations.find(curr);
            it != func.debugLocations.end()) {
          loc = it->second;
        }
      }
    } finder(func);
    finder.walk(func.body);
    if (finder.loc) {
      return finder.loc;
    }
    if (func.epilogLocation.size()) {
      return *func.epilogLocation.begin();
    }
    return std::nullopt;
  }

  void run(Module* module) override {
    std::map<std::string, std::set<std::string>> moduleFuncs;

    for (auto& func : module->functions) {
      auto loc = getFirstDebugLoc(*func);
      if (!loc) {
        continue;
      }
      auto mod = getModule(module->debugInfoFileNames[loc->fileIndex]);
      if (!mod) {
        continue;
      }
      moduleFuncs[*mod].insert(func->name.toString());
    }

    for (auto& [mod, funcs] : moduleFuncs) {
      std::cout << mod << '\n';
      for (auto func : funcs) {
        std::cout << func << '\n';
      }
      std::cout << '\n';
    }
  }
};

Pass* createClassifyFuncsPass() { return new ClassifyFuncs(); }

} // namespace wasm
