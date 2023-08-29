// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

// Code generated by tools/generate-trace-sources.cpp. DO NOT EDIT.
// source: src/quick-lint-js/logging/trace-types.h

#pragma once

#include <quick-lint-js/logging/trace-types.h>
#include <quick-lint-js/port/char8.h>
#include <string_view>

namespace quick_lint_js {
enum class Parsed_Trace_Event_Type {
  error_invalid_magic,
  error_invalid_uuid,
  error_unsupported_compression_mode,

  packet_header,

  error_unsupported_lsp_document_type,

  init_event,
  vscode_document_opened_event,
  vscode_document_closed_event,
  vscode_document_changed_event,
  vscode_document_sync_event,
  lsp_client_to_server_message_event,
  vector_max_size_histogram_by_owner_event,
  process_id_event,
  lsp_documents_event,
};

struct Parsed_Trace_Event {
  Parsed_Trace_Event_Type type;

  Trace_Event_Header header;

  union {
    // 'header' is not initialized for packet_header.
    Trace_Context packet_header;

    // The following have 'header' initialized.
    // clang-format off
    Trace_Event_Init init_event;
    Trace_Event_VSCode_Document_Opened<std::u16string_view> vscode_document_opened_event;
    Trace_Event_VSCode_Document_Closed<std::u16string_view> vscode_document_closed_event;
    Trace_Event_VSCode_Document_Changed<std::u16string_view> vscode_document_changed_event;
    Trace_Event_VSCode_Document_Sync<std::u16string_view> vscode_document_sync_event;
    Trace_Event_LSP_Client_To_Server_Message lsp_client_to_server_message_event;
    Trace_Event_Vector_Max_Size_Histogram_By_Owner vector_max_size_histogram_by_owner_event;
    Trace_Event_Process_ID process_id_event;
    Trace_Event_LSP_Documents lsp_documents_event;
    // clang-format on
  };
};
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
