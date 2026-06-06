from wit_world import exports
from wit_world.imports.wasi_http_types import OutgoingResponse, Fields, OutgoingBody, ResponseOutparam, Ok, IncomingRequest


class HttpHandler(exports.HttpHandler):
    def handle_request(self, request: IncomingRequest, response_out: ResponseOutparam) -> None:
        path = request.path_with_query() or "/"
        content = f"Hello, {path}!".encode()
        content_length = str(len(content)).encode()
        fields = Fields.from_list([("content-length", content_length)])
        out = OutgoingResponse(fields)
        body = out.body()
        with body.write() as stream:
            stream.blocking_write_and_flush(content)
        OutgoingBody.finish(body, None)
        ResponseOutparam.set(response_out, Ok(out))
