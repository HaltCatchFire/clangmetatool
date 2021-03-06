#ifndef INCLUDED_CLANGMETATOOL_META_TOOL_H
#define INCLUDED_CLANGMETATOOL_META_TOOL_H

#include <assert.h>
#include <map>
#include <memory>
#include <stddef.h>
#include <string>

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Core/Replacement.h>
#include <llvm/ADT/StringRef.h>

namespace clangmetatool {

  /**
   * MetaTool is a template that reduces the amount of boilerplate
   * required to write a clang tool. The WrappedTool is a class that
   * is expected to have:
   *
   *   * A constructor:
   *     - That takes as parameters:
   *       - A pointer to the compiler instance object.
   *       - A pointer to the MatchFinder object
   *     - Registers PPCallbacks on the preprocessor and/or
   *       add matchers to the match finder.
   *
   *   * A postProcessing method
   *     - That takes as parameter:
   *       - The replacementsMap for this run
   *
   */
  template <class WrappedTool>
  class MetaTool : public clang::ASTFrontendAction {
  private:
    std::map<std::string, clang::tooling::Replacements> &replacementsMap;
    clang::ast_matchers::MatchFinder f;
    WrappedTool *tool;
  public:
    MetaTool
    (std::map<std::string, clang::tooling::Replacements> &replacementsMap)
      :replacementsMap(replacementsMap), tool(NULL) { }

    ~MetaTool() {
      if (tool)
        delete tool;
    }

    virtual bool BeginSourceFileAction(clang::CompilerInstance &ci) override {
      // we don't expect to ever have the metatool be invoked more
      // than once, it would eventually result in us holding
      // references to unused compiler instance objects, and
      // eventually segfaulting, so assert here.
      assert(tool == NULL);
      tool = new WrappedTool(&ci, &f);
      return true;
    }

    virtual void EndSourceFileAction() override {
      tool->postProcessing(replacementsMap);
    }

    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &CI, 
                      llvm::StringRef file) override {
      return f.newASTConsumer();
    }
  };

}

#endif

// ----------------------------------------------------------------------------
// Copyright 2018 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
