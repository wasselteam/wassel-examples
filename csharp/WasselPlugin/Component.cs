using HttpPluginWorld;
using HttpPluginWorld.wit.imports.wasi.http.v0_2_10;

public class HttpHandlerImpl : HttpPluginWorld.wit.exports.wassel.foundation.IHttpHandler
{
    public static void HandleRequest(ITypes.IncomingRequest request, ITypes.ResponseOutparam responseOut)
    {
        var path = request.PathWithQuery() ?? "/";
        var content = System.Text.Encoding.UTF8.GetBytes($"Hello, {path}!");
        var contentLength = System.Text.Encoding.UTF8.GetBytes(content.Length.ToString());

        var headers = new ITypes.Fields();
        headers.Append("content-length", contentLength);

        var response = new ITypes.OutgoingResponse(headers);
        var body = response.Body();
        using (var stream = body.Write())
        {
            stream.BlockingWriteAndFlush(content);
        }

        ITypes.OutgoingBody.Finish(body, null);
        ITypes.ResponseOutparam.Set(responseOut, Result<ITypes.OutgoingResponse, ITypes.ErrorCode>.Ok(response));
    }
}
