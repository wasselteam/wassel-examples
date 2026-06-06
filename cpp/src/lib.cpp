#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

#include "http_plugin.h"

namespace plugin {

http_plugin_string_t to_plugin_string(std::string_view s) {
  auto *ptr = static_cast<uint8_t *>(malloc(s.size()));
  memcpy(ptr, s.data(), s.size());
  return {ptr, s.size()};
}

wasi_http_types_list_field_value_t make_field_values(std::string_view value) {
  auto *entry = static_cast<wasi_http_types_field_value_t *>(
      malloc(sizeof(wasi_http_types_field_value_t)));
  entry->ptr = reinterpret_cast<uint8_t *>(const_cast<char *>(value.data()));
  entry->len = value.size();
  return {entry, 1};
}

void set_header(wasi_http_types_borrow_fields_t fields, std::string_view name,
                std::string_view value) {
  auto field_name = to_plugin_string(name);
  auto values = make_field_values(value);

  wasi_http_types_header_error_t error;
  if (!wasi_http_types_method_fields_set(fields, &field_name, &values, &error))
    wasi_http_types_header_error_free(&error);

  http_plugin_string_free(&field_name);
  free(values.ptr);
}

wasi_http_types_own_fields_t build_headers(std::string_view content_length) {
  auto fields = wasi_http_types_constructor_fields();
  set_header(wasi_http_types_borrow_fields(fields), "content-length",
             content_length);
  return fields;
}

bool write_body(wasi_http_types_borrow_outgoing_body_t body_borrow,
                const std::string &content) {
  wasi_http_types_own_output_stream_t stream;
  if (!wasi_http_types_method_outgoing_body_write(body_borrow, &stream))
    return false;

  http_plugin_list_u8_t data{
      reinterpret_cast<uint8_t *>(const_cast<char *>(content.data())),
      content.size()};

  wasi_io_streams_stream_error_t error;
  bool ok = wasi_io_streams_method_output_stream_blocking_write_and_flush(
      wasi_io_streams_borrow_output_stream(stream), &data, &error);

  wasi_io_streams_output_stream_drop_own(stream);

  if (!ok)
    wasi_io_streams_stream_error_free(&error);

  return ok;
}

void finish_body(wasi_http_types_own_outgoing_body_t body) {
  wasi_http_types_error_code_t error;
  if (!wasi_http_types_static_outgoing_body_finish(body, nullptr, &error))
    wasi_http_types_error_code_free(&error);
}

wasi_http_types_own_outgoing_response_t
build_response(const std::string &content) {
  auto content_len = std::to_string(content.size());
  auto fields = build_headers(content_len);
  auto response = wasi_http_types_constructor_outgoing_response(fields);

  wasi_http_types_own_outgoing_body_t body;
  if (wasi_http_types_method_outgoing_response_body(
          wasi_http_types_borrow_outgoing_response(response), &body)) {
    write_body(wasi_http_types_borrow_outgoing_body(body), content);
    finish_body(body);
  }

  return response;
}

std::string get_path(wasi_http_types_borrow_incoming_request_t request) {
  http_plugin_string_t path;
  if (!wasi_http_types_method_incoming_request_path_with_query(request, &path))
    return "/";

  std::string result(reinterpret_cast<char *>(path.ptr), path.len);
  http_plugin_string_free(&path);
  return result;
}

void send_response(
    exports_wassel_foundation_http_handler_own_response_outparam_t response_out,
    wasi_http_types_own_outgoing_response_t response) {
  wasi_http_types_result_own_outgoing_response_error_code_t result{
      .is_err = false, .val = {.ok = response}};
  wasi_http_types_static_response_outparam_set(response_out, &result);
}

} // namespace plugin

extern "C" void exports_wassel_foundation_http_handler_handle_request(
    exports_wassel_foundation_http_handler_own_incoming_request_t request,
    exports_wassel_foundation_http_handler_own_response_outparam_t
        response_out) {
  auto borrow = wasi_http_types_borrow_incoming_request(request);
  auto path = plugin::get_path(borrow);
  auto content = "Hello, " + path + "!";

  plugin::send_response(response_out, plugin::build_response(content));

  wasi_http_types_incoming_request_drop_own(request);
}
