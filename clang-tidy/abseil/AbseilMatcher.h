//===- AbseilMatcher.h - clang-tidy ---------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <algorithm>

namespace clang {
namespace ast_matchers {

/// Matches AST nodes that were found within Abseil files.
///
/// Example matches Y but not X
///     (matcher = cxxRecordDecl(isInAbseilFile())
/// \code
///   #include "absl/strings/internal-file.h"
///   class X {};
/// \endcode
/// absl/strings/internal-file.h:
/// \code
///   class Y {};
/// \endcode
///
/// Usable as: Matcher<Decl>, Matcher<Stmt>, Matcher<TypeLoc>,
/// Matcher<NestedNameSpecifierLoc>
AST_POLYMORPHIC_MATCHER(
    isInAbseilFile, AST_POLYMORPHIC_SUPPORTED_TYPES(Decl, Stmt, TypeLoc,
                                                    NestedNameSpecifierLoc)) {
  auto &SourceManager = Finder->getASTContext().getSourceManager();
  SourceLocation Loc = Node.getBeginLoc();
  if (Loc.isInvalid())
    return false;
  const FileEntry *FileEntry =
      SourceManager.getFileEntryForID(SourceManager.getFileID(Loc));
  if (!FileEntry)
    return false;
  // Determine whether filepath contains "absl/[absl-library]" substring, where
  // [absl-library] is AbseilLibraries list entry.
  StringRef Path = FileEntry->getName();
  const static llvm::SmallString<5> AbslPrefix("absl/");
  size_t PrefixPosition = Path.find(AbslPrefix);
  if (PrefixPosition == StringRef::npos)
    return false;
  Path = Path.drop_front(PrefixPosition + AbslPrefix.size());
  static const char *AbseilLibraries[] = {"algorithm",
                                          "base",
                                          "container",
                                          "debugging",
                                          "flags"
                                          "memory",
                                          "meta",
                                          "numeric",
                                          "strings",
                                          "synchronization",
                                          "time",
                                          "types",
                                          "utility"};
  return std::any_of(
      std::begin(AbseilLibraries), std::end(AbseilLibraries),
      [&](const char *Library) { return Path.startswith(Library); });
}

} // namespace ast_matchers
} // namespace clang
