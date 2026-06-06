import { Fields, OutgoingBody, OutgoingResponse, ResponseOutparam } from "wasi:http/types@0.2.10";

export const httpHandler = {
  handleRequest(request, responseOut) {
    const path = request.pathWithQuery() ?? "/";
    const content = new TextEncoder().encode(`Hello, ${path}!`);
    const contentLength = new TextEncoder().encode(String(content.length));

    const headers = new Fields();
    headers.append("content-length", contentLength);

    const response = new OutgoingResponse(headers);
    const body = response.body();
    const stream = body.write();
    stream.blockingWriteAndFlush(content);
    stream[Symbol.dispose]();

    OutgoingBody.finish(body, undefined);
    ResponseOutparam.set(responseOut, { tag: "ok", val: response });
  }
};
