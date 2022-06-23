// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iterator>
#include <quick-lint-js/array.h>
#include <quick-lint-js/char8.h>
#include <quick-lint-js/cli-location.h>
#include <quick-lint-js/diag-collector.h>
#include <quick-lint-js/diag-matcher.h>
#include <quick-lint-js/diagnostic-types.h>
#include <quick-lint-js/language.h>
#include <quick-lint-js/padded-string.h>
#include <quick-lint-js/parse-support.h>
#include <quick-lint-js/parse.h>
#include <quick-lint-js/spy-visitor.h>
#include <quick-lint-js/string-view.h>
#include <string>
#include <string_view>
#include <vector>

using ::testing::ElementsAre;
using ::testing::IsEmpty;

namespace quick_lint_js {
namespace {
TEST(test_parse_typescript_type, direct_type_reference) {
  {
    spy_visitor v = parse_and_visit_typescript_type(u8"Type"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_type_use"));  // Type
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"Type"}));
  }
}

TEST(test_parse_typescript_type, namespaced_type_reference) {
  {
    spy_visitor v = parse_and_visit_typescript_type(u8"ns.Type"_sv);
    EXPECT_THAT(v.visits, ElementsAre("visit_variable_namespace_use"));  // ns
    EXPECT_THAT(v.variable_uses,
                ElementsAre(spy_visitor::visited_variable_use{u8"ns"}));
  }
}
}
}

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew "strager" Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.
