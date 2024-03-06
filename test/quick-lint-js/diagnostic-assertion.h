// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#pragma once

#include <cstddef>
#include <gmock/gmock.h>
#include <memory>
#include <quick-lint-js/container/fixed-vector.h>
#include <quick-lint-js/container/padded-string.h>
#include <quick-lint-js/container/result.h>
#include <quick-lint-js/diag/diag-list.h>
#include <quick-lint-js/diag/diagnostic-types.h>
#include <quick-lint-js/port/char8.h>
#include <quick-lint-js/port/span.h>
#include <quick-lint-js/util/cast.h>
#include <quick-lint-js/util/cpp.h>
#include <string>
#include <vector>

namespace quick_lint_js {
// NOTE[_diag-syntax]: Diagnostic_Assertion objects are created from a
// specification string:
//
//   u8"^^^ Diag_Unexpected_Token"_diag
//
// A specification has several parts:
//
// * (optional) Alignment and diagnostic span: Zero or more space characters
//   which position the diagnostic span, followed by one of the following:
//   * One or more '^' characters. Each '^' represents a code character that the
//     diagnostic covers.
//   * One '`' character. The '`' represents a diagnostic in between two code
//     characters. The '`' is positioned on the latter of the two code
//     characters.
// * Diagnostic type: A Diag_ class name.
// * (optional) Member variable: A field inside the Diag_ class. Written after
//   '.' after the diagnostic type. If the Diag_ class only has one member
//   variable, the member variable (including the '.') may be omitted.
// * (optional) Extra member variable and value: A field inside the Diag_ class
//   and its expected value. Written between '{' and '}' following the
//   diagnostic.
//
// Diagnostic_Assertion objects are to be used with
// test_parse_and_visit_statement and related functions.
//
// Here are some examples uses:
//
// clang-format off
//
//   // Parse the code and assert that there is a
//   // Diag_Unexpected_Comma_After_Class_Field diagnostic. The
//   // diagnostic's .comma is asserted to be a Source_Code_Span covering one
//   // character: the ','. .comma starts at offset 15 and ends at offset 16.
//   test_parse_and_visit_statement(
//      u8"class C { a = 1, b = 2 }"_sv,  //
//      u8"               ^ Diag_Unexpected_Comma_After_Class_Field"_diag);
//
//   // Parse the code and assert that there is a
//   // Diag_Missing_Function_Parameter_List diagnostic. The
//   // diagnostic's .expected_parameter_list is asserted to be a
//   // Source_Code_Span covering zero characters. .expected_parameter_list
//   // starts at offset 16 and ends at offset 16.
//   test_parse_and_visit_statement(
//      u8"class C { method { body; } }"_sv,  //
//      u8"                ` Diag_Missing_Function_Parameter_List"_diag);
//
//    // Parse the code and assert that there are two separate diagnostics.
//    test_parse_and_visit_statement(
//      u8"class C { if method(arg) { body; } instanceof myField; }"_sv,              //
//      u8"                                   ^^^^^^^^^^ Diag_Unexpected_Token"_diag, //
//      u8"          ^^ Diag_Unexpected_Token"_diag);
//
//    // Parse the code and assert that there is one diagnostic with two fields
//    // (one Source_Code_Span (.where) and one Char8 (.token)).
//    test_parse_and_visit_statement(
//      u8"do {} while (cond"_sv,
//      u8"                 ` Diag_Expected_Parenthesis_Around_Do_While_Condition.where{.token=)}"_diag);
//
// clang-format on
struct Diagnostic_Assertion {
  struct Member {
    String8_View name;
    std::uint8_t offset;
    Diagnostic_Arg_Type type = Diagnostic_Arg_Type::invalid;

    // If type == Diagnostic_Arg_Type::source_code_span:
    Padded_String_Size span_begin_offset;
    Padded_String_Size span_end_offset;

    // If type == Diagnostic_Arg_Type::char8:
    Char8 character;

    // If type == Diagnostic_Arg_Type::enum_kind:
    Enum_Kind enum_kind;

    // If type == Diagnostic_Arg_Type::string8_view:
    String8_View string;

    // If type == Diagnostic_Arg_Type::statement_kind:
    Statement_Kind statement_kind;

    // If type == Diagnostic_Arg_Type::variable_kind:
    Variable_Kind variable_kind;

    static Member make_span(String8_View name, std::uint8_t offset,
                            Padded_String_Size span_begin_offset,
                            Padded_String_Size span_end_offset) {
      Member m;
      m.name = name;
      m.offset = offset;
      m.type = Diagnostic_Arg_Type::source_code_span;
      m.span_begin_offset = span_begin_offset;
      m.span_end_offset = span_end_offset;
      return m;
    }
  };

  Diag_Type type = Diag_Type();
  Fixed_Vector<Member, 4> members;

  // Whether adjusted_for_escaped_characters should be called to compensate for
  // control characters.
  bool needs_adjustment = false;

  // If the specification is malformed, return a list of messages to report to
  // the user.
  //
  // Postcondition: return.needs_adjustment
  static Result<Diagnostic_Assertion, std::vector<std::string>> parse(
      const Char8* specification);

  // If the specification is malformed, exit the program.
  static Diagnostic_Assertion parse_or_exit(const Char8* specification);

  // Adjust span_begin_offset and span_end_offset based on characters in 'code'
  // which were probably escaped in the C++ source code.
  //
  // This function compensates for C++ escape sequences such as in the following
  // example:
  //
  //    test_parse_and_visit_statement(
  //      u8"\"string\""_sv,
  //      u8"     ^ MyDiag"_diag);
  //
  // The Diagnostic_Assertion should point to the letter 'i' in the input
  // string. The 'i' is at byte offset 4, but there are five spaces before the
  // '^' in the _diag string. adjusted_for_escaped_characters would subtract 1
  // from the offsets in the _diag string so that assert_diagnostics will work
  // correctly.
  //
  // If this->needs_adjustment is false, returns an unmodified copy of *this.
  //
  // Postcondition: !return.needs_adjustment
  //
  // TODO(strager): Support Unicode escape sequences (\u2063 for example).
  Diagnostic_Assertion adjusted_for_escaped_characters(String8_View code) const;

  // Manually create a Diagnostic_Assertion without the short-hand syntax.
  //
  // See DIAGNOSTIC_ASSERTION_SPAN.
  //
  // Postcondition: !return.needs_adjustment
  static Diagnostic_Assertion make_raw(Diag_Type,
                                       std::initializer_list<Member>);
};

// Create a Diagnostic_Assertion which matches 'type_'. It asserts that
// 'type_::member_' is a Source_Code_Span beginning at 'begin_offset_' and
// ending at 'begin_offset_ + span_string_.size()'.
//
// If you need to match two fields of the diagnostic type, see
// DIAGNOSTIC_ASSERTION_2_SPANS.
#define DIAGNOSTIC_ASSERTION_SPAN(type_, member_, begin_offset_, span_string_) \
  (::quick_lint_js::Diagnostic_Assertion::make_raw(                            \
      Diag_Type::type_,                                                        \
      {                                                                        \
          ::quick_lint_js::Diagnostic_Assertion::Member::make_span(            \
              QLJS_CPP_QUOTE_U8_SV(member_), offsetof(type_, member_),         \
              ::quick_lint_js::narrow_cast<Padded_String_Size>(                \
                  (begin_offset_)),                                            \
              ::quick_lint_js::narrow_cast<Padded_String_Size>(                \
                  (begin_offset_)) +                                           \
                  ::quick_lint_js::narrow_cast<Padded_String_Size>(            \
                      (span_string_).size())),                                 \
      }))

// Create a Diagnostic_Assertion which matches 'type_'.
//
// It asserts that 'type_::member_0_' is a Source_Code_Span beginning at
// 'begin_offset_0_' and ending at 'begin_offset_0_ + span_string_0_.size()'.
//
// It asserts that 'type_::member_1_' is a Source_Code_Span beginning at
// 'begin_offset_1_' and ending at 'begin_offset_1_ + span_string_1_.size()'.
#define DIAGNOSTIC_ASSERTION_2_SPANS(type_, member_0_, begin_offset_0_,    \
                                     span_string_0_, member_1_,            \
                                     begin_offset_1_, span_string_1_)      \
  (::quick_lint_js::Diagnostic_Assertion::make_raw(                        \
      Diag_Type::type_,                                                    \
      {                                                                    \
          ::quick_lint_js::Diagnostic_Assertion::Member::make_span(        \
              QLJS_CPP_QUOTE_U8_SV(member_0_), offsetof(type_, member_0_), \
              ::quick_lint_js::narrow_cast<Padded_String_Size>(            \
                  (begin_offset_0_)),                                      \
              ::quick_lint_js::narrow_cast<Padded_String_Size>(            \
                  (begin_offset_0_)) +                                     \
                  ::quick_lint_js::narrow_cast<Padded_String_Size>(        \
                      (span_string_0_).size())),                           \
          ::quick_lint_js::Diagnostic_Assertion::Member::make_span(        \
              QLJS_CPP_QUOTE_U8_SV(member_1_), offsetof(type_, member_1_), \
              ::quick_lint_js::narrow_cast<Padded_String_Size>(            \
                  (begin_offset_1_)),                                      \
              ::quick_lint_js::narrow_cast<Padded_String_Size>(            \
                  (begin_offset_1_)) +                                     \
                  ::quick_lint_js::narrow_cast<Padded_String_Size>(        \
                      (span_string_1_).size())),                           \
      }))

// See [_diag-syntax].
//
// Exits the program at run-time if the specification is malformed.
Diagnostic_Assertion operator""_diag(const Char8* specification,
                                     std::size_t specification_length);

void assert_diagnostics(Padded_String_View code, const Diag_List& diagnostics,
                        Span<const Diagnostic_Assertion> assertions,
                        Source_Location caller);
void assert_diagnostics(Padded_String_View code, const Diag_List& diagnostics,
                        std::initializer_list<Diagnostic_Assertion> assertions,
                        Source_Location caller = Source_Location::current());

::testing::Matcher<const Diag_List&> diagnostics_matcher_2(
    Padded_String_View code, Span<const Diagnostic_Assertion> assertions);
::testing::Matcher<const Diag_List&> diagnostics_matcher_2(
    Padded_String_View code,
    std::initializer_list<Diagnostic_Assertion> assertions);

template <class Diag>
Diag* get_only_diagnostic(const Diag_List& diags, Diag_Type type) {
  Diag* diag = nullptr;
  int found_count = 0;
  diags.for_each([&](Diag_Type current_type, void* raw_diag) -> void {
    if (current_type == type) {
      ++found_count;
      diag = static_cast<Diag*>(raw_diag);
    }
  });
  return found_count == 1 ? diag : nullptr;
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
